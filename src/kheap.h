/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

/**
 * Initializes the kernel heap. The extents and location
 * of this heap are fixed at a specific region.
 *
 * @see KHEAP_START
 * @see KHEAP_END
 */
void kheap_init();

/**
 * Allocates a chunk of memory of bytes size.
 * The returned block is guaranteed to be aligned to at
 * least 4 bytes (8 bytes on 64 bit architectures).
 *
 * @param bytes the number of bytes to allocate.
 * @return      a pointer to a memory block.
 */
void* kheap_alloc(size_t bytes);

/**
 * Resizes a given memory chunk by either really extending
 * the block if room is there, or otherwise allocating a
 * large enough block, moving the contents there, and freeing
 * the old memory.
 *
 * @param mem   the old memory block.
 * @param bytes the new block size in bytes.
 * @return      pointer to the resized block.
 */
void* kheap_realloc(void* mem, size_t bytes);

/**
 * Deallocates a previously allocated block of memory.
 *
 * @param mem   the block to deallocate.
 */
void kheap_free(void* mem);

