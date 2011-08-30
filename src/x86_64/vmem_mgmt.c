/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include <vmem.h>
#include <vmem_mgmt.h>
#include <log.h>
#include <pmem.h>
#include <mem.h>

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
