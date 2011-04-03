/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "vmem.h"
#include "vmem_mgmt.h"
#include "pmem.h"
#include "log.h"
#include "mem.h"

#include <x86/paging.h>

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
