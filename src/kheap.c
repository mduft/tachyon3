/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "kheap.h"
#include "vmem.h"
#include "pmem.h"
#include "aspace.h"
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
#define KHEAP_BL_DATASZ(b)  ((b) & KHEAP_FLMASK)

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
#define KHEAP_BL_H2F(h) ((kheap_block_t*)((uintptr_t)(h) + sizeof(kheap_block_t) + (KHEAP_BL_DATASZ((*h)))))

/**
 * Retrieve the block header from the block footer.
 *
 * @param f pointer to the footer
 */
#define KHEAP_BL_F2H(f) ((kheap_block_t*)((uintptr_t)(f) - sizeof(kheap_block_t) - (KHEAP_BL_DATASZ((*f)))))

/**
 * Retrieve a pointer to the actual user data from a block header
 *
 * @param h pointer to the header.
 */
#define KHEAP_BL_H2M(h) ((void*)((uintptr_t)(h) + sizeof(kheap_block_t)))

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
    if(KHEAP_BL_DATASZ(*hdr) != KHEAP_BL_DATASZ(*ftr)) {
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

    if(!vmem_map(aspace_current(), phys, (void*)KHEAP_START, KHEAP_PG_FLAGS)) {
        fatal("error in kernel heap: cannot map virtual memory!\n");
    }

    /* TODO: mark the entire first page as free block */
}

void* kheap_alloc(size_t bytes) {
    return NULL;
}

void kheap_free(void* mem) {
    if(!kheap_validate(mem)) {
        error("kernel heap block validation failed for %p!\n", mem);
    } else {

    }
}

void* kheap_realloc(void* mem, size_t bytes) {
    /* TODO: try to resize current chunk. or is trying to find a
     * fit more expesive then re-allocating all the time? */

    if(!kheap_validate(mem)) {
        error("kernel heap block validation failed for %p!\n", mem);
        return NULL;
    }

    void* new = kheap_alloc(bytes);
    memmove(new, mem, KHEAP_BL_DATASZ(*KHEAP_BL_M2H(mem)));
    kheap_free(mem);
    return new;
}
