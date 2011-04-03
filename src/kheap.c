/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "kheap.h"
#include "vmem.h"
#include "pmem.h"
#include "spc.h"
#include "mem.h"
#include "log.h"

#include <x86/paging.h>

/**
 * The paging flags used by the kernel heap.
 */
#define KHEAP_PG_FLAGS  (PG_WRITABLE | PG_GLOBAL)

#define KHEAP_PRESENT   0x1 /**< whether a block is allocated or free */
#define KHEAP_FOOTER    0x2 /**< whether the current location is a header or footer */
#define KHEAP_FLMASK    0x3 /**< used to mask off all flags from a kheap_block_t */

/**
 * Determine the size of a memory block in user usable bytes
 * (thus excluding header and footer).
 *
 * @param b kheap_block_t, either a header, or a footer.
 */
#define KHEAP_BL_B2DATASZ(b)    ((b) & ~KHEAP_FLMASK)

/**
 * Calculate the user data's memory block size from the management
 * block size (including header + footer)
 *
 * @param b the size of the management block.
 */
#define KHEAP_BL_SZ2DATASZ(b)   ((b) - (sizeof(kheap_block_t) * 2))

/**
 * Calculate the management block size from the user data's block
 * size (excluding the header + footer)
 *
 * @param b the size of the user data block.
 */
#define KHEAP_BL_DATASZ2SZ(b)   ((b) + (sizeof(kheap_block_t) * 2))

/**
 * Retrieve the block header from a pointer to the block data itself.
 *
 * @param m a pointer to a memory block.
 */
#define KHEAP_BL_M2H(m) ((kheap_block_t*)((uintptr_t)(m) - sizeof(kheap_block_t)))

/**
 * Retrieve the block footer from the block header.
 *
 * @param h pointer to the header
 */
#define KHEAP_BL_H2F(h) ((kheap_block_t*)((uintptr_t)(h) + sizeof(kheap_block_t) + (KHEAP_BL_B2DATASZ((*h)))))

/**
 * Retrieve the block header from the block footer.
 *
 * @param f pointer to the footer
 */
#define KHEAP_BL_F2H(f) ((kheap_block_t*)((uintptr_t)(f) - sizeof(kheap_block_t) - (KHEAP_BL_B2DATASZ((*f)))))

/**
 * Retrieve a pointer to the actual user data from a block header
 *
 * @param h pointer to the header.
 */
#define KHEAP_BL_H2M(h) ((void*)((uintptr_t)(h) + sizeof(kheap_block_t)))

/**
 * Get the address of the next block header. Whether a next block
 * actually exists has to be checked first!
 *
 * @param h pointer to the current header.
 */
#define KHEAP_BL_NEXT(h)    (KHEAP_BL_H2F(h) + 1)

/**
 * Get the address of the previous block header. Whether a previous
 * block actually exists has to be checked first!
 *
 * @param h pointer to the current header.
 */
#define KHEAP_BL_PREV(h)    (KHEAP_BL_F2H((h - 1)))

/**
 * Verify that a given block has a successor.
 *
 * @param h pointer to the header
 */
#define KHEAP_BL_HAS_NEXT(h)    (!(((uintptr_t)KHEAP_BL_NEXT(h)) >= kheap_state.vmem_mark))

/**
 * Verify that a given block has a predecessor.
 *
 * @param h pointer to the header
 */
#define KHEAP_BL_HAS_PREV(h)    (((uintptr_t)h) > KHEAP_START)

/**
 * Updates a block with the specified values.
 *
 * @param h pointer to the header.
 * @param s new size of the block.
 * @param f flags (KHEAP_PRESENT).
 */
#define KHEAP_BL_SET(h, s, f) \
    { *h = (s) | f; register kheap_block_t* ft = KHEAP_BL_H2F(h); *ft = (s) | f | KHEAP_FOOTER; }

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
typedef uintptr_t kheap_block_t;

/**
 * This global state stores the current virtual memory allocation
 * and mapping state, and some additional information about the
 * amounts of memory allocated.
 */
static struct {
    uintptr_t vmem_mark;    /**< the current virtual memory allocation mark */
    size_t used_bytes;      /**< the current count of allocated bytes */
} kheap_state;

/**
 * Validate, that a given memory address is a kernel heap allocated
 * (or free, actually) block.
 *
 * @param m the pointer to verify.
 */
static inline bool kheap_validate(void* m) {
    /* block must be properly aligned. */
    if((uintptr_t)m % sizeof(kheap_block_t)) {
        return false;
    }

    register kheap_block_t* hdr = KHEAP_BL_M2H(m);

    /* must not be a footer, must be a header. */
    if(!((uintptr_t)hdr >= KHEAP_START && (uintptr_t)hdr < KHEAP_END) || *hdr & KHEAP_FOOTER) {
        return false;
    }

    register kheap_block_t* ftr = KHEAP_BL_H2F(hdr);

    /* footer flag must be set on footer */
    if(!((uintptr_t)ftr > KHEAP_START && (uintptr_t)ftr <= KHEAP_END) || !(*ftr & KHEAP_FOOTER)) {
        return false;
    }

    /* header and footer must agree on size */
    if(KHEAP_BL_B2DATASZ(*hdr) != KHEAP_BL_B2DATASZ(*ftr)) {
        return false;
    }

    return true;
}

void kheap_init() {
    kheap_state.vmem_mark = KHEAP_START + PAGE_SIZE_4K;
    kheap_state.used_bytes = 0;

    phys_addr_t phys = pmem_alloc(PAGE_SIZE_4K, PAGE_SIZE_4K);

    if(!phys) {
        fatal("error in kernel heap: cannot allocate physical memory!\n");
    }

    if(!vmem_map(spc_current(), phys, (void*)KHEAP_START, KHEAP_PG_FLAGS)) {
        fatal("error in kernel heap: cannot map virtual memory!\n");
    }

    KHEAP_BL_SET((kheap_block_t*)KHEAP_START, KHEAP_BL_SZ2DATASZ(PAGE_SIZE_4K), 0);
}

void* kheap_alloc(size_t bytes) {
    bytes = ALIGN_UP(bytes, sizeof(kheap_block_t));

    register kheap_block_t* block = (kheap_block_t*)KHEAP_START;

    while((uintptr_t)block >= KHEAP_START 
            && (uintptr_t)block < kheap_state.vmem_mark) {

        if(!((*block) & KHEAP_PRESENT) && 
                KHEAP_BL_B2DATASZ(*block) >= KHEAP_BL_DATASZ2SZ(bytes)) {
            /* block is free and large enough, let's split it. */
            register size_t freesz = KHEAP_BL_B2DATASZ(*block) - KHEAP_BL_DATASZ2SZ(bytes);
            KHEAP_BL_SET(block, bytes, KHEAP_PRESENT);
            KHEAP_BL_SET(KHEAP_BL_NEXT(block), freesz, 0);

            kheap_state.used_bytes += bytes;
            return KHEAP_BL_H2M(block);
        }

        if(!KHEAP_BL_HAS_NEXT(block)) {
            /* no more room, allocate another page, and try again */
            register size_t sz = KHEAP_BL_B2DATASZ(*block);
            register size_t pcnt = ALIGN_UP(bytes, PAGE_SIZE_4K) / PAGE_SIZE_4K;

            for(register size_t i = 0; i < pcnt; ++i) {
                register phys_addr_t phys = pmem_alloc(PAGE_SIZE_4K, PAGE_SIZE_4K);

                if(!vmem_map(spc_current(), phys,
                    (void*)kheap_state.vmem_mark, KHEAP_PG_FLAGS))
                {
                    error("failed to allocate more room for the kernel heap!\n");
                    return NULL;
                } else {
                    kheap_state.vmem_mark += PAGE_SIZE_4K;
                    KHEAP_BL_SET(block, sz + (PAGE_SIZE_4K * pcnt), 0);
                }
            }

            /* re-check the current block (which has just been resized)  */
            continue;
        }

        /* no matching block, and still blocks to search left over */
        block = KHEAP_BL_NEXT(block);
    }

    return NULL;
}

void kheap_free(void* mem) {
    if(!kheap_validate(mem)) {
        error("kernel heap block validation failed for %p!\n", mem);
    } else {
        register kheap_block_t* block = KHEAP_BL_M2H(mem); 
        register size_t actual_sz = KHEAP_BL_B2DATASZ(*block);

        if(KHEAP_BL_HAS_NEXT(block)) {
            register kheap_block_t* next = KHEAP_BL_NEXT(block);

            if(!((*next) & KHEAP_PRESENT)) {
                /* extend block over the next one (free) */
                actual_sz += KHEAP_BL_DATASZ2SZ(KHEAP_BL_B2DATASZ(*next));
            }
        }

        if(KHEAP_BL_HAS_PREV(block)) {
            register kheap_block_t* prev = KHEAP_BL_PREV(block);

            if(!((*prev) & KHEAP_PRESENT)) {
                /* extend block over the previous one (free) */
                actual_sz += KHEAP_BL_DATASZ2SZ(KHEAP_BL_B2DATASZ(*prev));
                block = prev;
            }
        }

        KHEAP_BL_SET(block, actual_sz, 0);
    }
}

void* kheap_realloc(void* mem, size_t bytes) {
    if(!kheap_validate(mem)) {
        error("kernel heap block validation failed for %p!\n", mem);
        return NULL;
    }

    register kheap_block_t* block = KHEAP_BL_M2H(mem);

    /* no shrinking ... */
    if(KHEAP_BL_B2DATASZ(*block) >= bytes)
        return mem;

    if(KHEAP_BL_HAS_NEXT(block)) {
        register kheap_block_t* next = KHEAP_BL_NEXT(block);

        if(!((*next) & KHEAP_PRESENT) && 
            (KHEAP_BL_DATASZ2SZ(KHEAP_BL_B2DATASZ(*next)) + 
             KHEAP_BL_B2DATASZ(*block)) >= bytes)
        {
            if(((ssize_t)KHEAP_BL_B2DATASZ(*next) - ((ssize_t)bytes - 
                (ssize_t)KHEAP_BL_B2DATASZ(*block))) <= 0) 
            {
                /* completely merge block */
                register size_t nsz = (KHEAP_BL_B2DATASZ(*block) + 
                    KHEAP_BL_DATASZ2SZ(KHEAP_BL_B2DATASZ(*next)));
                KHEAP_BL_SET(block, nsz, KHEAP_PRESENT);
            } else {
                register size_t nsz = KHEAP_BL_B2DATASZ(*next) - 
                    (bytes - KHEAP_BL_B2DATASZ(*block));
                KHEAP_BL_SET(block, bytes, KHEAP_PRESENT);
                next = KHEAP_BL_NEXT(block);
                KHEAP_BL_SET(next, nsz, 0);
            }
            return mem;
        }
    }

    void* new = kheap_alloc(bytes);
    memmove(new, mem, KHEAP_BL_B2DATASZ(*block));
    kheap_free(mem);
    return new;
}
