/* Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

/**
 * This state stores the current virtual memory allocation
 * and mapping state, and some additional information about the
 * amounts of memory allocated.
 */
typedef struct {
    bool valid;             /**< true if the heap is valid. */
    uintptr_t vmem_mark;    /**< the current virtual memory allocation mark */
    size_t used_bytes;      /**< the current count of allocated bytes */
    size_t block_count;     /**< count of allocated blocks */
} heap_state_t;

/**
 * Describes a heap instance.
 */
typedef struct {
    uintptr_t start;    /**< the virtual start of the heap */
    uintptr_t end;      /**< the virtual end of the heap */
    uintptr_t pg_fl;    /**< the paging flags used by this heap */
    spc_t space;        /**< the address space this heap is in */
    heap_state_t state; /**< storage for the internal heap state */
} heap_t;

/**
 * Initializes a new heap. The extents and location
 * of this heap are fixed at the given region.
 *
 * @param heap  the target descriptor. it has to be
 *              pre-initialized with all required values
 *              (start, end, pg_fl, space)
 * @return      the new heap descriptor
 */
bool heap_init(heap_t* heap);

/**
 * Deletes a previously created heap, freeing any
 * associated allocated blocks.
 *
 * @param heap  the heap to destroy
 * @return      always NULL
 */
heap_t* heap_delete(heap_t* heap);

/**
 * Allocates a chunk of memory of bytes size.
 * The returned block is guaranteed to be aligned to at
 * least 4 bytes (8 bytes on 64 bit architectures).
 *
 * @param heap  the heap to allocate from.
 * @param bytes the number of bytes to allocate.
 * @return      a pointer to a memory block.
 */
void* heap_alloc(heap_t* heap, size_t bytes);

/**
 * Deallocates a previously allocated block of memory.
 *
 * @param heap  the heap to free the block in.
 * @param mem   the block to deallocate.
 */
void heap_free(heap_t* heap, void* mem);

/**
 * Resizes a given memory chunk by either really extending
 * the block if room is there, or otherwise allocating a
 * large enough block, moving the contents there, and freeing
 * the old memory.
 *
 * @param heap  the heap to allocate from.
 * @param mem   the old memory block.
 * @param bytes the new block size in bytes.
 * @return      pointer to the resized block.
 */
void* heap_realloc(heap_t* heap, void* mem, size_t bytes);
