/* Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

#define VM_SPLIT_ALLOC  0x1     /**< allocate structure if they don't exist. */
#define VM_SPLIT_LARGE  0x2     /**< operate on large pages, not small */
#define VM_SPLIT_USER   0x4     /**< verify (and upgrade if neccessary!) usermode access on structures */

/**
 * Invalidate a given virtual address, meaning the CPU should remove
 * any accordign entries from TLBs.
 */
#define VM_INVAL(x)         asm volatile("invlpg (%0)" :: "r"((x)))

typedef enum {
    TableNotMapped,
    SmallExpected,
    LargeExpected,
    SplitSuccess,
    SplitError
} vmem_split_res_t;

/**
 * The mapspace, that is used to temporarily map page management 
 * structures, as they are required. This is architecure-implementation
 * defined, and thus is located in the architecture specific code.
 */
extern phys_addr_t* ps_mapspace;

/**
 * Maps a paging structure into the kernel address space, which in
 * turn is mapped into all other spaces.
 *
 * @param phys  the physical address to map.
 * @return      the mapped virtual address.
 */
void* vmem_mgmt_map(phys_addr_t phys);

/**
 * Removes a temporary mapping from the temporary mapspace.
 *
 * @param virt  the virtual address to unmap.
 */
void vmem_mgmt_unmap(void* virt);

/**
 * Allocate a paging structure from the physical memory. This also
 * clears the memory to all zeros, so it can be used immediately
 * for virtual memory management.
 *
 * @return the physical address of the paging structure.
 */
phys_addr_t vmem_mgmt_alloc();

/**
 * Frees a previously allocated paging structure. Currently only
 * calls pmem_free(addr).
 *
 * @param addr  the address to free.
 */
void vmem_mgmt_free(phys_addr_t addr);

/**
 * Splits a virtual address in it's components, and maps all the
 * according management structure to the temporary mapspace.
 *
 * The caller is responsible for calling vmem_mgmt_unmap for each
 * of the mapped structures.
 *
 * @param space     the address space to use.
 * @param virt      the virtual address to process.
 * @param[out] pd   the page directory.
 * @param[out] pt   the page table. NULL if VM_SPLIT_LARGE.
 * @param[out] ipd  the index into the PD to find the PT or large page.
 * @param[out] ipt  the index into the PT to find the page (only if pt != NULL!)
 * @param flags     can be either of:   
 *                      - VM_SPLIT_ALLOC: allocate new structures.
 *                          otherwise the address must be mapped.
 *                      - VM_SPLIT_LARGE: the virtual address should
 *                          be treated as a large page.
 * @param pgflags   additional paging flags to assure present on the structures.
 *                  will only be evaluated if VM_SPLIT_ALLOC is given.
 */
vmem_split_res_t vmem_mgmt_split(spc_t space, uintptr_t virt, 
        uintptr_t** pd, uintptr_t** pt, size_t* ipd, size_t* ipt, uint32_t flags, uint32_t pgflags);

/**
 * Prepares a given address space by putting all kernel global
 * mappings in it.
 *
 * @param space the address space to manipulate.
 * @return      true on success, false otherwise.
 */
bool vmem_mgmt_make_glob_spc(spc_t space);

/**
 * Completely clobbers a given address space, unmapping all pages, and freeing all associated memory.
 *
 * Note that global kernel mappings are left intact, of course.
 *
 * @param space the address space to clobber.
 */
void vmem_mgmt_clobber_spc(spc_t space);

/**
 * Adds an additional global memory region to be mapped in
 * every single address space. this is then added to every
 * new address space created with spc_new.
 *
 * Parameters are the same as vmem_map, except that you don't
 * have to specify a target address space.
 */
void vmem_mgmt_add_global_mapping(phys_addr_t phys, void* virt, uint32_t flags);

/**
 * Dumps the mappings that the given space contains.
 *
 * @param space the address space to dump
 */
void vmem_mgmt_dump_spc(spc_t space);
