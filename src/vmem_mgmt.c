/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "vmem.h"
#include "vmem_mgmt.h"
#include "pmem.h"
#include "log.h"
#include "mem.h"
#include "list.h"
#include "kheap.h"

#include "paging.h"

/* defined in paging.S */
extern phys_addr_t x86_64_pg_tmap;
extern phys_addr_t x86_64_pg_kernel_pdpt;
extern phys_addr_t x86_64_pg_pt_low;

phys_addr_t* ps_mapspace = (phys_addr_t*)((uintptr_t)&x86_64_pg_tmap + CORE_VMA_X86_64);

typedef struct {
    phys_addr_t from;
    void* to;
    uint32_t flags;
} vmem_glob_mapping_t;

static list_t* vmem_glob_map = 0; 

#define TRACE_FLAGS 1

#ifdef TRACE_FLAGS
# define VM_TRACE_FLAGS(space, x, u) {                                      \
        char buf[8];                                                        \
        vmem_mgmt_gen_flag_string(buf, u);                                  \
        trace("pg-flags for " #x ", spc=%p: 0x%x (%s)\n", space, u, buf);   \
    }
#else
# define VM_TRACE_FLAGS(space, x, u)
#endif

#define VM_CHECK_MAPPING(x) \
    { if(!(x)) { error("cannot map " #x " for %p\n", virt); } }

#define VM_CHECK_ENTRY(x, i, u) \
    if(!((x)[i] & PG_PRESENT)) {                                    \
        if(flags & VM_SPLIT_ALLOC) {                                \
            (x)[i] = vmem_mgmt_alloc() | PG_PRESENT | PG_WRITABLE;  \
        } else {                                                    \
            goto not_found;                                         \
        }                                                           \
    }                                                               \
    if(flags & VM_SPLIT_ALLOC) {                                    \
        VM_TRACE_FLAGS(space, x, u);                                \
        (x)[i] |= u;                                                \
    }

/** 
 * length of fixed mapspace for paging structures.
 *
 * @attention: this has to be kept in sync with the definition in
 *             paging.S, or trouble lies ahead.
 */
#define VM_PS_MAPSPACE_SZ   0xA

/**
 * Size of a single paging structure, used to manage the different
 * levels of virtual memory mappings. Typically the size of a small
 * page (4K).
 */
#define VM_PS_STRUCT_SZ     PAGE_SIZE_4K

/**
 * Converts an index into the temporary mapspace to a virtual address.
 * It is assumed, that the temporary mapspace is the very last in the
 * virtual address space (so this macro can calculate from ~0x0 on).
 */
#define VM_PS_TADDR(idx)    (void*)(((((uintptr_t)~0x0ull) & ~(VM_PS_STRUCT_SZ - 1)) - ((VM_PS_MAPSPACE_SZ -1) * VM_PS_STRUCT_SZ)) + (idx * VM_PS_STRUCT_SZ))

/**
 * Helper that wraps arounf VM_PAGE, to make code more readable.
 */
#define VM_PS_PAGE(x)       VM_PAGE(x, VM_PS_STRUCT_SZ)

/**
 * Defines a mask for interesting flags to determine whether a mapped
 * page fulfills all requirements to use it.
 */
#define VM_FL_MASK          (PG_WRITABLE | PG_USER | PG_WRITETHROUGH \
                            | PG_NONCACHABLE | PG_GLOBAL)

/**
 * Calculates the actual interesting flags for a complete page table entry
 */
#define VM_FLAGS(x)         (((x) & ~VM_ENTRY_FLAG_MASK) & VM_FL_MASK)

static void vmem_mgmt_gen_flag_string(char buf[8], uintptr_t flags) {
    buf[0] = (flags & PG_WRITABLE) ? 'w' : '-';
    buf[1] = (flags & PG_USER) ? 'u' : '-';
    buf[2] = (flags & PG_WRITETHROUGH) ? 't' : '-';
    buf[3] = (flags & PG_NONCACHABLE) ? 'n' : '-';
    buf[4] = (flags & PG_GLOBAL) ? 'g' : '-';
    buf[5] = (flags & PG_PRESENT) ? 'p' : '-';
    buf[6] = (flags & ~(PG_GLOBAL | PG_WRITABLE | PG_USER | PG_WRITETHROUGH | PG_NONCACHABLE | PG_PRESENT)) != 0 ? '?' : '-';
    buf[7] = 0;
}

void* vmem_mgmt_map(phys_addr_t phys) {
    register phys_addr_t page = VM_PS_PAGE(phys);

    if(VM_OFFSET(phys, VM_PS_STRUCT_SZ) != 0) {
        fatal("misaligned physical address for paging structure.\n");
    }

    for(register size_t i = 0; i < VM_PS_MAPSPACE_SZ; ++i) {
        if(!(ps_mapspace[i] & PG_PRESENT)) {
            ps_mapspace[i] = page | PG_PRESENT | PG_WRITABLE | PG_GLOBAL;

            VM_INVAL(VM_PS_TADDR(i));
            return VM_PS_TADDR(i);
        }
    }

    fatal("out of ps_mapspace. this probably means there is a bug in vmem!\n");
}

void vmem_mgmt_unmap(void* virt) {
    for(register size_t i = 0; i < VM_PS_MAPSPACE_SZ; ++i) {
        if(VM_PS_TADDR(i) == virt) {
            ps_mapspace[i] = 0;

            VM_INVAL(virt);
            return;
        }
    }
}

phys_addr_t vmem_mgmt_alloc() {
    phys_addr_t phys = pmem_alloc(VM_PS_STRUCT_SZ, VM_PS_STRUCT_SZ);

    if(!phys) {
        fatal("out of physical memory for vmem_mgmt\n");
    }

    void* mapped = vmem_mgmt_map(phys);
    memset(mapped, 0, VM_PS_STRUCT_SZ);
    vmem_mgmt_unmap(mapped);

    return phys;
}

void vmem_mgmt_free(phys_addr_t addr) {
    pmem_free(addr, VM_PS_STRUCT_SZ);
}

vmem_split_res_t vmem_mgmt_split(spc_t space, uintptr_t virt, uintptr_t** pd, 
                    uintptr_t** pt, size_t* ipd, size_t* ipt, uint32_t flags, uint32_t pgflags) {
    if(!pd || !pt) {
        return Error;
    }

    vmem_split_res_t result = SplitError;

    register size_t idx_pml4 = (virt >> 39) & 0x1FF;
    register size_t idx_pdpt = (virt >> 30) & 0x1FF;
    register size_t idx_pd   = (virt >> 21) & 0x1FF;

    uintptr_t* pdpt = NULL;
    uintptr_t* pml4 = vmem_mgmt_map(space);

    // TODO: tweak flags to only contain allowed values for real CPUs on PDPTE PDE PTE

    VM_CHECK_MAPPING(pml4);
    VM_CHECK_ENTRY(pml4, idx_pml4, pgflags);

    pdpt = vmem_mgmt_map(pml4[idx_pml4] & VM_ENTRY_FLAG_MASK);
    VM_CHECK_MAPPING(pdpt);
    VM_CHECK_ENTRY(pdpt, idx_pdpt, pgflags);

    *pd = vmem_mgmt_map(pdpt[idx_pdpt] & VM_ENTRY_FLAG_MASK);
    VM_CHECK_MAPPING((*pd));

    if(flags & VM_SPLIT_LARGE) {
        if(((*pd)[idx_pd] & PG_PRESENT) && !((*pd)[idx_pd] & PG_LARGE)) {
            warn("large page requested, but present entry is a page table\n");
            result = LargeExpected;
            goto error;
        }
    } else {
        if(((*pd)[idx_pd] & PG_PRESENT) && ((*pd)[idx_pd] & PG_LARGE)) {
            warn("small page requested, but large page already mapped here\n");
            result = SmallExpected;
            goto error;
        }

        VM_CHECK_ENTRY((*pd), idx_pd, pgflags);

        *pt = vmem_mgmt_map((*pd)[idx_pd] & VM_ENTRY_FLAG_MASK);
        VM_CHECK_MAPPING((*pt));

        *ipt = (virt >> 12) & 0x1FF;
    }

    *ipd = idx_pd;

    vmem_mgmt_unmap(pml4);
    vmem_mgmt_unmap(pdpt);

    return SplitSuccess;

not_found:
    result = TableNotMapped;
error:
    if(pml4) { vmem_mgmt_unmap(pml4); }
    if(pdpt) { vmem_mgmt_unmap(pdpt); }
    if(*pd)  { vmem_mgmt_unmap(*pd);  }
    if(*pt)  { vmem_mgmt_unmap(*pt);  }
    return result;
}

bool vmem_mgmt_make_glob_spc(spc_t space) {
    uintptr_t* pml4 = (uintptr_t*)vmem_mgmt_map(space);

    if(!pml4) {
        error("failed to map pml4\n");
        return false;
    }

    // TODO: no easier (maintainable) way?
    uintptr_t ng_flags = (PG_KFLAGS & ~PG_GLOBAL);

    memset(pml4, 0, PAGE_SIZE_4K);
    pml4[511] = (uintptr_t)&x86_64_pg_kernel_pdpt | ng_flags;

    phys_addr_t phys_pdpt_low = pmem_alloc(PAGE_SIZE_4K, PAGE_SIZE_4K);
    pml4[0] = phys_pdpt_low | ng_flags;

    uintptr_t* pdpt = (uintptr_t*)vmem_mgmt_map(phys_pdpt_low);
    
    if(!phys_pdpt_low || !pdpt) {
        error("failed to map pdpt\n");
        return false;
    }

    memset(pdpt, 0, PAGE_SIZE_4K);

    phys_addr_t phys_pd_low = pmem_alloc(PAGE_SIZE_4K, PAGE_SIZE_4K);
    pdpt[0] = phys_pd_low | ng_flags;

    uintptr_t* pd = (uintptr_t*)vmem_mgmt_map(phys_pd_low);

    if(!phys_pd_low || !pd) {
        error("failed to map pd\n");
        return false;
    }

    memset(pd, 0, PAGE_SIZE_4K);
    pd[0] = (phys_addr_t)&x86_64_pg_pt_low | ng_flags;
    pd[1] = 0x200000 | ng_flags | PG_LARGE | PG_GLOBAL;

    vmem_mgmt_unmap(pd);
    vmem_mgmt_unmap(pdpt);
    vmem_mgmt_unmap(pml4);

    // now map all the registered global regions
    list_node_t* node = list_begin(vmem_glob_map);
    while(node) {
        vmem_glob_mapping_t* map = (vmem_glob_mapping_t*)node->data;

        if(map) {
            char buf[8];
            vmem_mgmt_gen_flag_string(buf, map->flags);
            trace("trying to map: %p -> %p (%s); old = %p\n", map->from, map->to, buf, vmem_resolve(space, map->to));

            // if there is a mapping already, thats ok, but it has to meet the flags.
            // an exception is, that a large page is ok if a small one was requested.
            size_t ipd, ipt;
            uintptr_t* pd;
            uintptr_t* pt;

            register uintptr_t pg_flags;
            register uintptr_t pg_rflags;
            
            switch(vmem_mgmt_split(space, (uintptr_t)map->to, &pd, &pt, &ipd, &ipt, 
                ((map->flags & PG_LARGE) ? VM_SPLIT_LARGE : 0), 0)) {
            case TableNotMapped:
                if(!vmem_map(space, map->from, map->to, map->flags))
                    fatal("cannot insert required global mapping %p -> %p!\n", 
                        map->from, map->to);
                break;
            case SmallExpected:
                // small page requested, but large found, thats the good case :)
                // fallthrough to the flag check.
            case SplitSuccess:
                // it's already present. check flags whether they match.

                // this time we want only the flags, nothing else.
                if(pd[ipd] & PG_LARGE) {
                    pg_flags = VM_FLAGS(pd[ipd]);
                    pg_rflags = pd[ipd] & ~VM_ENTRY_FLAG_MASK;
                } else {
                    pg_flags = VM_FLAGS(pt[ipt]);
                    pg_rflags = pt[ipt] & ~VM_ENTRY_FLAG_MASK;
                }

                if(pg_rflags & PG_PRESENT) {
                    if(pg_flags != VM_FLAGS(map->flags)) {
                        fatal("present page does not match flags, 0x%x (0x%x) != 0x%x\n", 
                            pg_flags, pg_rflags, VM_FLAGS(map->flags));
                    }
                } else {
                    if(!vmem_map(space, map->from, map->to, map->flags))
                        fatal("cannot insert required global mapping %p -> %p!\n", 
                            map->from, map->to);
                }

                // and finally unmap management stuff.
                if(pt) vmem_mgmt_unmap(pt);
                if(pd) vmem_mgmt_unmap(pd);
                break;
            case LargeExpected:
                // large expected, but small found .... :|
                fatal("cannot insert large page, small page already mapped at %p\n", map->to);
            case SplitError:
                fatal("cannot split virtual address %p ?!\n", map->to);
            }

        }

        node = node->next;
    }

    return true;
}

#define vmem_mgmt_clobber_helper(level) \
    for(register size_t i = 0; i < 512; ++i) { \
        if(!(level[i] & PG_PRESENT)) \
            continue; \
        phys_addr_t addr = level[i] & VM_ENTRY_FLAG_MASK;

#define vmem_mgmt_clobber_helper_chain(next) \
        uintptr_t* entry = (uintptr_t*)vmem_mgmt_map(addr); \
        if(addr) { \
            vmem_mgmt_clobber_ ## next(entry); \
            vmem_mgmt_unmap(entry); \
        } else { \
            error("problem while trying to destroy " #next "!\n"); \
        }

#define vmem_mgmt_clobber_helper_end(level) \
        pmem_free(addr, PAGE_SIZE_4K); \
        level[i] &= ~PG_PRESENT; \
    }

static void vmem_mgmt_clobber_pt(uintptr_t* pt) {
    vmem_mgmt_clobber_helper(pt);
    vmem_mgmt_clobber_helper_end(pt);
}

static void vmem_mgmt_clobber_pd(uintptr_t* pd) {
    if((pd[0] & VM_ENTRY_FLAG_MASK) == (phys_addr_t)&x86_64_pg_pt_low) {
        pd[0] = 0;
        pd[1] = 0;
    }

    vmem_mgmt_clobber_helper(pd);

    if(pd[i] & PG_LARGE) {
        phys_addr_t large_page = pd[i] & VM_ENTRY_FLAG_MASK;
        pmem_free(large_page, PAGE_SIZE_2M);
    } else {
        vmem_mgmt_clobber_helper_chain(pt);
    }

    vmem_mgmt_clobber_helper_end(pd);
}

static void vmem_mgmt_clobber_pdpt(uintptr_t* pdpt) {
    vmem_mgmt_clobber_helper(pdpt);
    vmem_mgmt_clobber_helper_chain(pd);
    vmem_mgmt_clobber_helper_end(pdpt);
}

void vmem_mgmt_clobber_spc(spc_t space) {
    uintptr_t* pml4 = (uintptr_t*)vmem_mgmt_map(space);
    vmem_mgmt_clobber_helper(pml4);

    if(addr == (phys_addr_t)&x86_64_pg_kernel_pdpt)
        continue;

    vmem_mgmt_clobber_helper_chain(pdpt);
    vmem_mgmt_clobber_helper_end(pml4);

    vmem_mgmt_unmap(pml4);
}

void vmem_mgmt_add_global_mapping(phys_addr_t phys, void* virt, uint32_t flags) {
    if(vmem_glob_map == 0) {
        vmem_glob_map = list_new();
    }

    vmem_glob_mapping_t* map = kheap_alloc(sizeof(vmem_glob_mapping_t));
    map->from = phys;
    map->to = virt;
    map->flags = flags;

    list_add(vmem_glob_map, map);
}

// TODO: refactor walking of the address space, so that dumping is just one of the possible use cases.

static void vmem_mgmt_dump_pt(uintptr_t virt_addr, uintptr_t pt_ptr) {
    uintptr_t* pt = vmem_mgmt_map(pt_ptr & VM_ENTRY_FLAG_MASK);

    if(!pt) {
        error("\t\t\tcannot map PT\n");
        return;
    }

    for(uintptr_t pte = 0; pte < 512; ++pte) {
        uintptr_t page = pt[pte];
        if(page & PG_PRESENT) {
            char buf[6];
            vmem_mgmt_gen_flag_string(buf, VM_FLAGS(page));
            trace("\t\t\tPAGE 4K %4d: %p -> %p (%s)\n", pte, virt_addr | (pte << 12), page & VM_ENTRY_FLAG_MASK, buf);
        }
    }

    vmem_mgmt_unmap(pt);
}

static void vmem_mgmt_dump_pd(uintptr_t parent_virt_addr, uintptr_t pd_ptr) {
    uintptr_t* pd = vmem_mgmt_map(pd_ptr & VM_ENTRY_FLAG_MASK);

    if(!pd) {
        error("\t\tcannot map PD\n");
        return;
    }

    for(uintptr_t pde = 0; pde < 512; ++pde) {
        uintptr_t pt_ptr = pd[pde];
        if(pt_ptr & PG_PRESENT) {
            uintptr_t virt_addr = parent_virt_addr | (pde << 21);
            char buf[6];
            vmem_mgmt_gen_flag_string(buf, VM_FLAGS(pt_ptr));
            if(pt_ptr & PG_LARGE) {
                trace("\t\tPAGE 2M %4d: %p -> %p (%s)\n", pde, virt_addr, pt_ptr & VM_ENTRY_FLAG_MASK, buf);
            } else {
                trace("\t\tPD %4d: %p (%s)\n", pde, pt_ptr & VM_ENTRY_FLAG_MASK, buf);
                vmem_mgmt_dump_pt(virt_addr, pt_ptr);
            }
        }
    }

    vmem_mgmt_unmap(pd);
}

static void vmem_mgmt_dump_pdpt(uintptr_t parent_virt_addr, uintptr_t pdpt_ptr) {
    uintptr_t* pdpt = vmem_mgmt_map(pdpt_ptr & VM_ENTRY_FLAG_MASK);

    if(!pdpt) {
        error("\tcannot map PDPT\n");
        return;
    }

    for(uintptr_t pdpte = 0; pdpte < 512; ++pdpte) {
        uintptr_t pd_ptr = pdpt[pdpte];
        if(pd_ptr & PG_PRESENT) {
            uintptr_t virt_addr = parent_virt_addr | (pdpte << 30);
            char buf[6];
            vmem_mgmt_gen_flag_string(buf, VM_FLAGS(pd_ptr));
            if(pd_ptr & PG_LARGE) {
                trace("\tPAGE 1G %4d: %p -> %p (%s)\n", pdpte, virt_addr, pd_ptr & VM_ENTRY_FLAG_MASK, buf);
            } else {
                trace("\tPDPT %4d: %p (%s)\n", pdpte, pd_ptr & VM_ENTRY_FLAG_MASK, buf);
                vmem_mgmt_dump_pd(virt_addr, pd_ptr);
            }
        }
    }

    vmem_mgmt_unmap(pdpt);
}

void vmem_mgmt_dump_spc(spc_t space) {
    uintptr_t* pml4 = vmem_mgmt_map(space);

    if(!pml4) {
        error("cannot map PML4!\n");
        return;
    }

    for(uintptr_t pml4e = 0; pml4e < 512; ++pml4e) {
        uintptr_t pdpt_ptr = pml4[pml4e];
        if(pdpt_ptr & PG_PRESENT) {
            uintptr_t virt_addr = pml4e << 39;
            char buf[6];
            vmem_mgmt_gen_flag_string(buf, VM_FLAGS(pdpt_ptr));
            if((virt_addr >> 47) & 1)
                virt_addr |= 0xffff000000000000;
            trace("PML4 %4d: %p (%s)\n", pml4e, pdpt_ptr & VM_ENTRY_FLAG_MASK, buf);
            vmem_mgmt_dump_pdpt(virt_addr, pdpt_ptr);
        }
    }

    vmem_mgmt_unmap(pml4);
}
