/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include <vmem.h>
#include <vmem_mgmt.h>
#include <log.h>

#include <x86/paging.h>

/* defined in paging.S */
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

