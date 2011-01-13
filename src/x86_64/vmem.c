/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include <vmem.h>
#include <pmem.h>

#include <x86/paging.h>

#define VM_SPLIT_ALLOC  0x1     /**< allocate structure if they don't exist. */
#define VM_SPLIT_LARGE  0x2     /**< operate on large pages, not small */

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
#define VM_PS_TADDR(idx)    (((~0x0ull & ~(PAGE_SIZE_2M - 1)) - ((VM_PS_MAPSPACE_SZ -1) * PAGE_SIZE_2M)) + (x * PAGE_SIZE_2M))

/**
 * Invalidate a given virtual address, meaning the CPU should remove
 * any accordign entries from TLBs.
 */
#define VM_INVAL(x)         asm volatile("invlpg (%0)" :: "r"((x)))

/* defined in paging.S */
extern phys_addr_t x86_64_pg_pml4;
extern phys_addr_t x86_64_pg_kernel_pdpt;
extern phys_addr_t x86_64_pg_tmap;

/**
 * The temporary mapspace, that is used to temporarily map page
 * management structures, as they are required.
 */
static phys_addr_t* ps_mapspace = &x86_64_pg_tmap;

/**
 * Maps a paging structure into the kernel address space, which in
 * turn is mapped into all other spaces.
 *
 * @param phys  the physical address to map.
 * @return      the mapped virtual address.
 */
static inline void* vmem_ps_map(phys_addr_t phys) {
    return ps_mapspace; /* TODO */
}

/**
 * Removes a temporary mapping from the temporary mapspace.
 *
 * @param virt  the virtual address to unmap.
 */
static inline void vmem_ps_unmap(void* virt) {

}

/**
 * Allocate a paging structure from the physical memory. This also
 * clears the memory to all zeros, so it can be used immediately
 * for virtual memory management.
 *
 * @return the physical address of the paging structure.
 */
static inline phys_addr_t vmem_ps_alloc() {
    return 0; /* TODO */
}

/**
 * Frees a previously allocated paging structure. Currently only
 * calls pmem_free(addr).
 *
 * @param addr  the address to free.
 */
static inline void vmem_ps_free(phys_addr_t addr) {

}

/**
 * Splits a virtual address in it's components, and maps all the
 * according management structure to the temporary mapspace.
 *
 * The caller is responsible for calling vmem_ps_unmap for each
 * of the mapped structures.
 *
 * @param space     the address space to use.
 * @param virt      the virtual address to process.
 * @param[out] pd   the page directory.
 * @param[out] pt   the page table. NULL if VM_SPLIT_LARGE.
 * @param flags     can be either of:   
 *                      - VM_SPLIT_ALLOC: allocate new structures.
 *                          otherwise the address must be mapped.
 *                      - VM_SPLIT_LARGE: the virtual address should
 *                          be treated as a large page.
 */
static inline bool vmem_ps_split(aspace_t space, uintptr_t virt, 
        uintptr_t** pd, uintptr_t** pt, uint32_t flags) {
    
    return 0; /* TODO */
}
