/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

/**
 * Defines a physical memory extension point.
 */
typedef struct {
    /** The count of regions defined by this extension point */
    size_t reg_cnt;

    /** 
     * The callback from which the region information can
     * be retrieved. The callback will be called reg_cnt
     * times with increasing index. 
     */
    bool (*reg_cb)(size_t index, uint64_t* start, uint64_t* len);
} pmem_ext_t;

/** extension point function type */
typedef pmem_ext_t (*pmem_ext_func_t)();

/**
 * Initializes the memory management. Since the three
 * memory management units in the kernel (pmem, vmem
 * and kheap) are tightly coupled, and initialization
 * if them relies on each other, this also calls
 * kheap_init() to bring up the whole memory subsystem.
 * (there is no vmem_init(), otherwise it would be called
 * from here or kheap_init() too).
 */
void pmem_init();

/**
 * Adds a new region of physical ram to the allocator.
 * The region must be contigous. If the regions extents
 * cross those of existing regions, it is shortened to
 * fit in the free area.
 *
 * @param start     the beginning physical address.
 * @param length    the length of the region in bytes.
 */
void pmem_add(phys_addr_t start, size_t length);

/**
 * Allocates a contigous chunk of physical memory with
 * the given length and alignment. The minimum alignment
 * is currently 4096 bytes.
 * This should probably only be used in conjunktion with
 * the virtual memory management facilities.
 *
 * @param length    the number of bytes to allocate.
 * @param align     the minimum alignment of the block.
 * @return          the address of the allocated block if
 *                  successfull, 0 (zero) otherwise.
 */
phys_addr_t pmem_alloc(size_t length, off_t align);

/**
 * Reserves the physical memory at the given location.
 * Fails if any of the pages in the given range have already
 * been allocated by somebody else.
 *
 * @attention Success does not mean, that the memory
 *            is actually physically available, but
 *            only means, that no pmem_alloc() call
 *            will ever return a block within the
 *            given range, regardless of how physical
 *            memory looks like. This is useful for
 *            reserving regions for memory mapped
 *            devices like the APIC, IOAPIC, etc.
 *
 * @param addr      the start of the region. if this is
 *                  not on a page boundary, it is aligned
 *                  down to the next lower page.
 * @param length    the length to reserve in bytes.
 * @return          true on success, false otherwise.
 */
bool pmem_reserve(phys_addr_t addr, size_t length);

/**
 * Frees a previously allocated block of physical memory.
 * Since the physical memory management does not track
 * allocations, the length needs to match the length given
 * to pmem_alloc!
 *
 * @param addr      the address of the block to free.
 * @param length    the length of the block.
 */
void pmem_free(phys_addr_t addr, size_t length);
