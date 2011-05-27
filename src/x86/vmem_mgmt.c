/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include <vmem.h>
#include <vmem_mgmt.h>
#include <log.h>
#include <pmem.h>
#include <mem.h>

#include <x86/paging.h>

/* defined in paging.S */
extern phys_addr_t x86_pg_pt;
extern phys_addr_t x86_pg_pt_high;
extern phys_addr_t x86_pg_tmap;

phys_addr_t* ps_mapspace = (phys_addr_t*)((uintptr_t)&x86_pg_tmap + CORE_VMA_X86);

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

bool vmem_mgmt_split(spc_t space, uintptr_t virt, uintptr_t** pd, 
                    uintptr_t** pt, size_t* ipd, size_t* ipt, uint32_t flags) {
    if(!pd || !pt) {
        return false;
    }

    register size_t idx_pd   = (virt >> 22) & 0x3FF;

    *pd = vmem_mgmt_map(space);
    VM_CHECK_MAPPING((*pd));

    if((*pd)[idx_pd] & PG_LARGE || flags & VM_SPLIT_LARGE) {
        if(flags & VM_SPLIT_LARGE 
                && (*pd[idx_pd] & PG_PRESENT) 
                && !(*pd[idx_pd] & PG_LARGE)) {
            error("large page requested, but present entry is a page table\n");
            goto error;
        }
    } else {
        VM_CHECK_ENTRY((*pd), idx_pd);

        *pt = vmem_mgmt_map((*pd)[idx_pd] & VM_ENTRY_FLAG_MASK);
        VM_CHECK_MAPPING((*pt));

        *ipt = (virt >> 12) & 0x3FF;
    }

    *ipd = idx_pd;

    return true;

error:
    if(*pd)  { vmem_mgmt_unmap(*pd);  }
    if(*pt)  { vmem_mgmt_unmap(*pt);  }
    return false;
}

bool vmem_mgmt_make_glob_spc(spc_t space) {
    uintptr_t* pd = (uintptr_t*)vmem_mgmt_map(space);

    if(!pd) {
        error("failed to map pd\n");
        return false;
    }

    uintptr_t ng_flags = (PG_KFLAGS & ~PG_GLOBAL);

    memset(pd, 0, PAGE_SIZE_4K);
    pd[0] = (uintptr_t)&x86_pg_pt | ng_flags;
    pd[0x300] = (uintptr_t)&x86_pg_pt_high | ng_flags;
    pd[0x3FF] = (uintptr_t)&x86_pg_tmap | ng_flags;

    vmem_mgmt_unmap(pd);
    return true;
}

void vmem_mgmt_clobber_spc(spc_t space) {
    uintptr_t* pd = (uintptr_t*)vmem_mgmt_map(space);
    
    if(!pd) {
        error("failed to map pd\n");
        return;
    }

    for(register size_t i = 0; i < 1024; ++i) {
        // skip kernel mappings.
        if(i == 0 || i == 0x300 || i == 0x3FF) {
            continue;
        }

        if(pd[i] & PG_PRESENT) {
            phys_addr_t addr = pd[i] & VM_ENTRY_FLAG_MASK;
            uintptr_t* pt = (uintptr_t*)vmem_mgmt_map(addr);

            for(register size_t j = 0; j < 1024; ++j) {
                if(pt[j] & PG_PRESENT) {
                    phys_addr_t paddr = pt[j] & VM_ENTRY_FLAG_MASK;
                    pmem_free(paddr, PAGE_SIZE_4K);
                }
            }

            vmem_mgmt_unmap(pt);
            pmem_free(addr, PAGE_SIZE_4K);
        }
    }

    vmem_mgmt_unmap(pd);
}
