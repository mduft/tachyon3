/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "vmem.h"
#include "vmem_mgmt.h"
#include "pmem.h"
#include "log.h"
#include "mem.h"

#include "paging.h"

/* defined in paging.S */
extern phys_addr_t x86_64_pg_tmap;
extern phys_addr_t x86_64_pg_kernel_pdpt;
extern phys_addr_t x86_64_pg_pt_low;

phys_addr_t* ps_mapspace = (phys_addr_t*)((uintptr_t)&x86_64_pg_tmap + CORE_VMA_X86_64);

#define VM_CHECK_MAPPING(x) \
    { if(!(x)) { error("cannot map " #x " for %p\n", virt); } }

#define VM_CHECK_ENTRY(x, i) \
    if(!((x)[i] & PG_PRESENT)) {                                    \
        if(flags & VM_SPLIT_ALLOC) {                                \
            (x)[i] = vmem_mgmt_alloc() | PG_PRESENT | PG_WRITABLE;  \
        } else {                                                    \
            goto error;                                             \
        }                                                           \
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

bool vmem_mgmt_split(spc_t space, uintptr_t virt, uintptr_t** pd, 
                    uintptr_t** pt, size_t* ipd, size_t* ipt, uint32_t flags) {
    if(!pd || !pt) {
        return false;
    }

    register size_t idx_pml4 = (virt >> 39) & 0x1FF;
    register size_t idx_pdpt = (virt >> 30) & 0x1FF;
    register size_t idx_pd   = (virt >> 21) & 0x1FF;

    uintptr_t* pdpt = NULL;
    uintptr_t* pml4 = vmem_mgmt_map(space);
    VM_CHECK_MAPPING(pml4);
    VM_CHECK_ENTRY(pml4, idx_pml4);

    pdpt = vmem_mgmt_map(pml4[idx_pml4] & VM_ENTRY_FLAG_MASK);
    VM_CHECK_MAPPING(pdpt);
    VM_CHECK_ENTRY(pdpt, idx_pdpt);

    *pd = vmem_mgmt_map(pdpt[idx_pdpt] & VM_ENTRY_FLAG_MASK);
    VM_CHECK_MAPPING((*pd));

    if(flags & VM_SPLIT_LARGE) {
        if(((*pd)[idx_pd] & PG_PRESENT) && !((*pd)[idx_pd] & PG_LARGE)) {
            error("large page requested, but present entry is a page table\n");
            goto error;
        }
    } else {
        if(((*pd)[idx_pd] & PG_PRESENT) && ((*pd)[idx_pd] & PG_LARGE)) {
            error("small page requested, but large page already mapped here\n");
            goto error;
        }

        VM_CHECK_ENTRY((*pd), idx_pd);

        *pt = vmem_mgmt_map((*pd)[idx_pd] & VM_ENTRY_FLAG_MASK);
        VM_CHECK_MAPPING((*pt));

        *ipt = (virt >> 12) & 0x1FF;
    }

    *ipd = idx_pd;

    vmem_mgmt_unmap(pml4);
    vmem_mgmt_unmap(pdpt);

    return true;

error:
    if(pml4) { vmem_mgmt_unmap(pml4); }
    if(pdpt) { vmem_mgmt_unmap(pdpt); }
    if(*pd)  { vmem_mgmt_unmap(*pd);  }
    if(*pt)  { vmem_mgmt_unmap(*pt);  }
    return false;
}

bool vmem_mgmt_make_glob_spc(spc_t space) {
    uintptr_t* pml4 = (uintptr_t*)vmem_mgmt_map(space);

    if(!pml4) {
        error("failed to map pml4\n");
        return false;
    }

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
    pd[1] = 0x200000 | ng_flags | PG_LARGE;

    vmem_mgmt_unmap(pd);
    vmem_mgmt_unmap(pdpt);
    vmem_mgmt_unmap(pml4);

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
