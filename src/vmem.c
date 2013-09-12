/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "vmem.h"
#include "vmem_mgmt.h"
#include "log.h"
#include "spc.h"
#include "extp.h"

#include <paging.h>

#define VM_RES_UNMAP(x, i) \
        result = x[i] & VM_ENTRY_FLAG_MASK;  \
        if(unmap) { x[i] = 0; }

#define VM_SET_IFN_PRESENT(x, i) \
        if(x[i] & PG_PRESENT) {                          \
            error("page allready present for %p\n", virt);  \
            result = false;                                 \
        } else { x[i] = phys | PG_PRESENT | flags; }

static inline phys_addr_t vmem_find_unmap(spc_t spc, void* virt, bool unmap) {
    size_t ipd, ipt;
    uintptr_t* pd;
    uintptr_t* pt;

    register phys_addr_t result = 0;

    vmem_split_res_t split = vmem_mgmt_split(spc, (uintptr_t)virt, &pd, &pt, &ipd, &ipt, 0, 0);
    
    switch(split) {
    case SplitSuccess:
        break;
    case TableNotMapped:
        return 0;
    case SmallExpected:
        // was a large page, but small expected - upgrade :)
        switch(vmem_mgmt_split(spc, (uintptr_t)virt, &pd, &pt, &ipd, &ipt, VM_SPLIT_LARGE, 0)) {
        case SplitSuccess:
            break;
        default:
            fatal("second chance failure during virtual memory mapping lookup\n");
        }
        break;
    default:
        fatal("unexpected error during virtual memory mapping lookup\n");
    }

    if(pd[ipd] & PG_LARGE) {
        VM_RES_UNMAP(pd, ipd);
    } else if(pt) {
        VM_RES_UNMAP(pt, ipt);

        vmem_mgmt_unmap(pt);
    } else {
        error("not a large page, but page table missing for %p!\n", virt);
    }

    if(unmap) {
        VM_INVAL(virt);
    }

    vmem_mgmt_unmap(pd);
    return result;
}

bool vmem_map(spc_t spc, phys_addr_t phys, void* virt, uint32_t flags) {
    size_t ipd, ipt;
    uintptr_t* pd;
    uintptr_t* pt;
    bool result = true;

    switch(vmem_mgmt_split(spc, (uintptr_t)virt, &pd, &pt, &ipd, &ipt, 
            VM_SPLIT_ALLOC | (flags & PG_LARGE ? VM_SPLIT_LARGE : 0), flags)) {
    case SplitSuccess:
    case TableNotMapped: // can not happen with VM_SPLIT_ALLOC, just for completeness
        break;
    default:
        return false;
    }

    /* TODO: are large pages actually supported on the current arch? */

    if(flags & PG_LARGE) {
        VM_SET_IFN_PRESENT(pd, ipd);
    } else if(pt) {
        VM_SET_IFN_PRESENT(pt, ipt);
    } else {
        error("not a large page, but page table missing fot %p!\n", virt);
        result = false;
    }

    if(pt) vmem_mgmt_unmap(pt);
    if(pd) vmem_mgmt_unmap(pd);
    return result;
}

phys_addr_t vmem_unmap(spc_t spc, void* virt) {
    return vmem_find_unmap(spc, virt, true);
}

phys_addr_t vmem_resolve(spc_t spc, void* virt) {
    return vmem_find_unmap(spc, virt, false);
}

