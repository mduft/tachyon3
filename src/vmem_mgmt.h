/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

#define VM_SPLIT_ALLOC  0x1     /**< allocate structure if they don't exist. */
#define VM_SPLIT_LARGE  0x2     /**< operate on large pages, not small */

/**
 * Invalidate a given virtual address, meaning the CPU should remove
 * any accordign entries from TLBs.
 */
#define VM_INVAL(x)         asm volatile("invlpg (%0)" :: "r"((x)))

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
 */
bool vmem_mgmt_split(spc_t space, uintptr_t virt, 
        uintptr_t** pd, uintptr_t** pt, size_t* ipd, size_t* ipt, uint32_t flags);

