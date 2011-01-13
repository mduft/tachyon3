/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include <vmem.h>
#include <vmem_ps.h>
#include <pmem.h>
#include <log.h>

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
#define VM_PS_TADDR(idx)    (((~0x0ull & ~(VM_PS_STRUCT_SZ - 1)) - ((VM_PS_MAPSPACE_SZ -1) * VM_PS_STRUCT_SZ)) + (idx * VM_PS_STRUCT_SZ))

/* defined in paging.S */
extern phys_addr_t x86_64_pg_pml4;
extern phys_addr_t x86_64_pg_kernel_pdpt;
extern phys_addr_t x86_64_pg_tmap;

/**
 * The temporary mapspace, that is used to temporarily map page
 * management structures, as they are required.
 */
static phys_addr_t* ps_mapspace = &x86_64_pg_tmap;

void* vmem_ps_map(phys_addr_t phys) {
    register phys_addr_t page = VM_PAGE(phys, VM_PS_STRUCT_SZ);

    if(VM_OFFSET(phys, VM_PS_STRUCT_SZ) != 0) {
        fatal("misaligned physical address for paging structure.\n");
    }

    for(register size_t i = 0; i < VM_PS_MAPSPACE_SZ; ++i) {
        if(!(ps_mapspace[i] & PG_PRESENT)) {
            ps_mapspace[i] = page | PG_PRESENT | PG_LARGE | PG_WRITABLE | PG_GLOBAL;

            VM_INVAL(VM_PS_TADDR(i));
            return (void*)VM_PS_TADDR(i);
        }
    }

    fatal("out of ps_mapspace. this probably means there is a bug in vmem!\n");
}

void vmem_ps_unmap(void* virt) {
    
}

phys_addr_t vmem_ps_alloc() {
    return 0; /* TODO */
}

void vmem_ps_free(phys_addr_t addr) {

}

bool vmem_ps_split(aspace_t space, uintptr_t virt, 
        uintptr_t** pd, uintptr_t** pt, uint32_t flags) {
    
    return 0; /* TODO */
}
