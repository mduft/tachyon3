/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "heap.h"
#include "vmem.h"
#include "pmem.h"
#include "spc.h"
#include "mem.h"
#include "log.h"

#include <x86/paging.h>

#define HEAP_PRESENT   0x1 /**< whether a block is allocated or free */
#define HEAP_FOOTER    0x2 /**< whether the current location is a header or footer */
#define HEAP_FLMASK    0x3 /**< used to mask off all flags from a heap_block_t */

/**
 * Determine the size of a memory block in user usable bytes
 * (thus excluding header and footer).
 *
 * @param b heap_block_t, either a header, or a footer.
 */
#define HEAP_BL_B2DATASZ(b)    ((b) & ~HEAP_FLMASK)

/**
 * Calculate the user data's memory block size from the management
 * block size (including header + footer)
 *
 * @param b the size of the management block.
 */
#define HEAP_BL_SZ2DATASZ(b)   ((b) - (sizeof(heap_block_t) * 2))

/**
 * Calculate the management block size from the user data's block
 * size (excluding the header + footer)
 *
 * @param b the size of the user data block.
 */
#define HEAP_BL_DATASZ2SZ(b)   ((b) + (sizeof(heap_block_t) * 2))

/**
 * Retrieve the block header from a pointer to the block data itself.
 *
 * @param m a pointer to a memory block.
 */
#define HEAP_BL_M2H(m) ((heap_block_t*)((uintptr_t)(m) - sizeof(heap_block_t)))

/**
 * Retrieve the block footer from the block header.
 *
 * @param h pointer to the header
 */
#define HEAP_BL_H2F(h) ((heap_block_t*)((uintptr_t)(h) + sizeof(heap_block_t) + (HEAP_BL_B2DATASZ((*h)))))

/**
 * Retrieve the block header from the block footer.
 *
 * @param f pointer to the footer
 */
#define HEAP_BL_F2H(f) ((heap_block_t*)((uintptr_t)(f) - sizeof(heap_block_t) - (HEAP_BL_B2DATASZ((*f)))))

/**
 * Retrieve a pointer to the actual user data from a block header
 *
 * @param h pointer to the header.
 */
#define HEAP_BL_H2M(h) ((void*)((uintptr_t)(h) + sizeof(heap_block_t)))

/**
 * Get the address of the next block header. Whether a next block
 * actually exists has to be checked first!
 *
 * @param h pointer to the current header.
 */
#define HEAP_BL_NEXT(h)    (HEAP_BL_H2F(h) + 1)

/**
 * Get the address of the previous block header. Whether a previous
 * block actually exists has to be checked first!
 *
 * @param h pointer to the current header.
 */
#define HEAP_BL_PREV(h)    (HEAP_BL_F2H((h - 1)))

/**
 * Verify that a given block has a successor.
 *
 * @param h pointer to the header
 */
#define HEAP_BL_HAS_NEXT(heap, h)    (!(((uintptr_t)HEAP_BL_NEXT(h)) >= heap->state.vmem_mark))

/**
 * Verify that a given block has a predecessor.
 *
 * @param h pointer to the header
 */
#define HEAP_BL_HAS_PREV(heap, h)    (((uintptr_t)h) > heap->start)

/**
 * Updates a block with the specified values.
 *
 * @param h pointer to the header.
 * @param s new size of the block.
 * @param f flags (HEAP_PRESENT).
 */
#define HEAP_BL_SET(h, s, f) \
    { *h = (s) | f; register heap_block_t* ft = HEAP_BL_H2F(h); *ft = (s) | f | HEAP_FOOTER; }

/**
 * Perform critical operational checks on this heap to
 * validate integrity.
 *
 * @param h the heap to check.
 */
#define HEAP_CHECK(h) \
    if(h->space != spc_current()) \
        fatal("the heap can only operate if the heaps address space is active!\n"); \


/**
 * The header of a heap memory block. If bit 0 is set, the block
 * is allocated, otherwise it is free. If bit 1 is set, the
 * information block points to a footer.
 *
 *  M (63 or 31)                        2 1 0
 * +-------------------------------------+-+-+
 * | blocksize                           |f|p|
 * +-------------------------------------+-+-+
 */
typedef uintptr_t heap_block_t;

/**
 * Validate, that a given memory address is a kernel heap allocated
 * (or free, actually) block.
 *
 * @param m the pointer to verify.
 */
static inline bool heap_validate(heap_t* heap, void* m) {
    /* block must be properly aligned. */
    if((uintptr_t)m % sizeof(heap_block_t)) {
        return false;
    }

    register heap_block_t* hdr = HEAP_BL_M2H(m);

    /* must not be a footer, must be a header. */
    if(!((uintptr_t)hdr >= heap->start && (uintptr_t)hdr < heap->end) || *hdr & HEAP_FOOTER) {
        return false;
    }

    register heap_block_t* ftr = HEAP_BL_H2F(hdr);

    /* footer flag must be set on footer */
    if(!((uintptr_t)ftr > heap->start && (uintptr_t)ftr <= heap->end) || !(*ftr & HEAP_FOOTER)) {
        return false;
    }

    /* header and footer must agree on size */
    if(HEAP_BL_B2DATASZ(*hdr) != HEAP_BL_B2DATASZ(*ftr)) {
        return false;
    }

    return true;
}

bool heap_init(heap_t* heap) {
    if(!heap || heap->start == 0 || heap->end == 0 || heap->space == 0)
        return false;

    HEAP_CHECK(heap);

    heap->state.vmem_mark = heap->start + PAGE_SIZE_4K;
    heap->state.used_bytes = 0;
    heap->state.block_count = 0;

    phys_addr_t phys = pmem_alloc(PAGE_SIZE_4K, PAGE_SIZE_4K);

    if(!phys) {
        error("error in heap: cannot allocate physical memory!\n");
        return false;
    }

    if(!vmem_map(heap->space, phys, (void*)heap->start, heap->pg_fl)) {
        error("error in heap: cannot map virtual memory!\n");
        return false;
    }

    HEAP_BL_SET((heap_block_t*)heap->start, HEAP_BL_SZ2DATASZ(PAGE_SIZE_4K), 0);

    heap->state.valid = true;

    return true;
}

heap_t* heap_delete(heap_t* heap) {
    // TODO: deallocate all!
    heap->state.valid = false;
    return NULL;
}

void* heap_alloc(heap_t* heap, size_t bytes) {
    HEAP_CHECK(heap);

    bytes = ALIGN_UP(bytes, sizeof(heap_block_t));

    register heap_block_t* block = (heap_block_t*)heap->start;

    while((uintptr_t)block >= heap->start 
            && (uintptr_t)block < heap->state.vmem_mark) {

        if(!((*block) & HEAP_PRESENT) && 
                HEAP_BL_B2DATASZ(*block) >= HEAP_BL_DATASZ2SZ(bytes)) {
            /* block is free and large enough, let's split it. */
            register size_t freesz = HEAP_BL_B2DATASZ(*block) - HEAP_BL_DATASZ2SZ(bytes);
            HEAP_BL_SET(block, bytes, HEAP_PRESENT);
            HEAP_BL_SET(HEAP_BL_NEXT(block), freesz, 0);

            heap->state.used_bytes += bytes;
            heap->state.block_count++;
            return HEAP_BL_H2M(block);
        }

        if(!HEAP_BL_HAS_NEXT(heap, block)) {
            /* no more room, allocate another page, and try again */
            register size_t sz = HEAP_BL_B2DATASZ(*block);
            register size_t pcnt = ALIGN_UP(bytes, PAGE_SIZE_4K) / PAGE_SIZE_4K;

            for(register size_t i = 0; i < pcnt; ++i) {
                register phys_addr_t phys = pmem_alloc(PAGE_SIZE_4K, PAGE_SIZE_4K);

                if(!vmem_map(heap->space, phys,
                    (void*)heap->state.vmem_mark, heap->pg_fl))
                {
                    error("failed to allocate more room for the kernel heap!\n");
                    return NULL;
                } else {
                    heap->state.vmem_mark += PAGE_SIZE_4K;
                    HEAP_BL_SET(block, sz + (PAGE_SIZE_4K * (i + 1)), 0);
                }
            }

            /* re-check the current block (which has just been resized)  */
            continue;
        }

        /* no matching block, and still blocks to search left over */
        block = HEAP_BL_NEXT(block);
    }

    return NULL;
}

void heap_free(heap_t* heap, void* mem) {
    HEAP_CHECK(heap);

    if(!heap_validate(heap, mem)) {
        error("kernel heap block validation failed for %p!\n", mem);
    } else {
        register heap_block_t* block = HEAP_BL_M2H(mem); 
        register size_t actual_sz = HEAP_BL_B2DATASZ(*block);

        heap->state.used_bytes -= actual_sz;
        heap->state.block_count--;

        if(HEAP_BL_HAS_NEXT(heap, block)) {
            register heap_block_t* next = HEAP_BL_NEXT(block);

            if(!((*next) & HEAP_PRESENT)) {
                /* extend block over the next one (free) */
                actual_sz += HEAP_BL_DATASZ2SZ(HEAP_BL_B2DATASZ(*next));
            }
        }

        if(HEAP_BL_HAS_PREV(heap, block)) {
            register heap_block_t* prev = HEAP_BL_PREV(block);

            if(!((*prev) & HEAP_PRESENT)) {
                /* extend block over the previous one (free) */
                actual_sz += HEAP_BL_DATASZ2SZ(HEAP_BL_B2DATASZ(*prev));
                block = prev;
            }
        }

        HEAP_BL_SET(block, actual_sz, 0);
    }
}

void* heap_realloc(heap_t* heap, void* mem, size_t bytes) {
    if(mem == NULL) {
        return heap_alloc(heap, bytes);
    }

    HEAP_CHECK(heap);

    if(!heap_validate(heap, mem)) {
        error("kernel heap block validation failed for %p!\n", mem);
        return NULL;
    }

    register heap_block_t* block = HEAP_BL_M2H(mem);

    /* no shrinking ... */
    if(HEAP_BL_B2DATASZ(*block) >= bytes)
        return mem;

    size_t orig_sz = HEAP_BL_B2DATASZ(*block);

    if(HEAP_BL_HAS_NEXT(heap, block)) {
        register heap_block_t* next = HEAP_BL_NEXT(block);

        if(!((*next) & HEAP_PRESENT) && 
            (HEAP_BL_DATASZ2SZ(HEAP_BL_B2DATASZ(*next)) + 
             HEAP_BL_B2DATASZ(*block)) >= bytes)
        {
            if(((ssize_t)HEAP_BL_B2DATASZ(*next) - ((ssize_t)bytes - 
                (ssize_t)HEAP_BL_B2DATASZ(*block))) <= 0) 
            {
                /* completely merge block */
                register size_t nsz = (HEAP_BL_B2DATASZ(*block) + 
                    HEAP_BL_DATASZ2SZ(HEAP_BL_B2DATASZ(*next)));
                HEAP_BL_SET(block, nsz, HEAP_PRESENT);
                heap->state.used_bytes += (nsz - orig_sz);
            } else {
                register size_t nsz = HEAP_BL_B2DATASZ(*next) - 
                    (bytes - HEAP_BL_B2DATASZ(*block));
                HEAP_BL_SET(block, bytes, HEAP_PRESENT);
                next = HEAP_BL_NEXT(block);
                HEAP_BL_SET(next, nsz, 0);
                heap->state.used_bytes += (bytes - orig_sz);
            }
            return mem;
        }
    }

    void* new = heap_alloc(heap, bytes);
    memmove(new, mem, HEAP_BL_B2DATASZ(*block));
    heap_free(heap, mem);
    return new;
}

