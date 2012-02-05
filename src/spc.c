/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include <spc.h>
#include <pmem.h>
#include <vmem_mgmt.h>
#include <log.h>
#include "paging.h"
#include "uapi.h"

spc_t spc_new() {
    // FIXME: would need far less room for a pml4 (64-bit only!)
    spc_t sp = (spc_t)pmem_alloc(PAGE_SIZE_4K, PAGE_SIZE_4K);

    if(sp == 0) {
        /* zero is reserved in any case */
        error("cannot create new address space!\n");
        return 0;
    }

    /* we need to stick in the correct global mappings */
    if(!vmem_mgmt_make_glob_spc(sp)) {
        error("cannot insert global mappings into new address space!\n");
        pmem_free(sp, PAGE_SIZE_4K);
        return 0;
    }

    /* we also need to enable the UAPI mappings in the new space */
    uapi_map(sp);

    return sp;
}

void spc_delete(spc_t target) {
    vmem_mgmt_clobber_spc(target);
    pmem_free(target, PAGE_SIZE_4K);
}

spc_t spc_current() {
    register spc_t sp asm("eax");
    asm volatile("mov %%cr3, %0" : "=a"(sp));
    return sp;
}

void spc_switch(spc_t target) {
    if(target == spc_current())
        return;

    asm volatile("mov %0, %%cr3" :: "r"(target));
}
