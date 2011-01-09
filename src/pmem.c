/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "pmem.h"
#include "bmap.h"
#include "list.h"

#define PMEM_PAGESIZE   4096
#define PMEM_FDEG       80
#define PMEM_PAGES(len) (ALIGN_UP(len, PMEM_PAGESIZE) / PMEM_PAGESIZE)

#define PMEM_TO_REGBIT(r, a)    ((ALIGN_DN(a, PMEM_PAGESIZE) - r->start) / PMEM_PAGESIZE)
#define PMEM_FROM_REGBIT(r, a)  (r->start + ((a) * PMEM_PAGESIZE))

/**
 * Each of those descriptors is searched from a list.
 * The bitmaps filling degree determines whether the
 * region is to be searched in the normal allocation
 * path. If memory runs low, all regions are searched.
 */
typedef struct {
    bitmap_t* bmap;     /**< the bitmap denoting page level allocation */
    phys_addr_t start;  /**< the lowest page's address managed by this region */
    size_t length;      /**< the length in bytes of this region */
} pmem_region_t;

/** The list of physical memory region descriptors */
static list_t reg_list;

/** This list node holds the descriptor of the bootstrap memory */
static listnode_t first_node;

/** This defines the struct to hold the initial memory region */
static pmem_region_t first_region;

/** 
 * This bitmap holds the individual bits for each page of
 * bootstrap memory 
 */
static bitmap_t first_bmap;

/**
 * This represents the real storage for the bitmap above.
 * it can hold bits for up to 4MB of physical RAM, which
 * is the bare minimum that must be available at boot.
 */
static uint32_t first_storage[32];

/**
 * Checks whether a given adress lies within a region.
 *
 * @param reg   the region to check.
 * @param addr  the address to check.
 */
static inline bool reg_contains(pmem_region_t* reg, phys_addr_t addr) {
    return (addr >= reg->start && addr <= (reg->start + reg->length));
}

void pmem_init() {
    reg_list.head = &first_node;
    reg_list.head->next = NULL;
    reg_list.head->payload = (uintptr_t)&first_region;

    bmap_init(&first_bmap, first_storage, (sizeof(first_storage) * 8));

    first_region.bmap = &first_bmap;
    first_region.start = 0;
    first_region.length = first_bmap.bits * PMEM_PAGESIZE;
}

void pmem_add(phys_addr_t start, size_t length) {
    /* TODO: this needs to be able to allocate from the
     * kernel heap, so we need virtual memory first! */
}

/**
 * Helper to really allocate memory either with filling degree
 * hinting on or off. only used from the real pmem_alloc.
 *
 * @param length    the block length to allocate
 * @param align     the minimum alignment for the block.
 * @param use_fdeg  whether to take filling degree into
 *                  account to speed up free region search.
 * @return          the allocate address, or 0 on failure.
 */
static phys_addr_t pmem_alloc_helper(size_t length, off_t align, bool use_fdeg) {
    listnode_t* current = reg_list.head;

    while(current) {
        pmem_region_t* reg = (pmem_region_t*)current->payload;

        /* make sure there is enough free room in the region to
         * fit the block to be allocated. */
        if(((use_fdeg && bmap_fdeg(reg->bmap) < PMEM_FDEG) || !use_fdeg)
                && ((reg->length * bmap_fdeg(reg->bmap))) > (length * 100)) {
            size_t idx;

            /* TODO: lock this allocation. permormance consideration:
             * should we lock the whole loop, or is locking/unlocking
             * fast enough? */

            if(bmap_search(reg->bmap, &idx, 0, PMEM_PAGES(length), (align / PMEM_PAGESIZE), BMAP_SRCH_HINTED)) {
                if(bmap_fill(reg->bmap, 1, idx, idx + PMEM_PAGES(length))) {
                    return PMEM_FROM_REGBIT(reg, idx);
                }
            }

            /* TODO: unlock */
        }

        current = current->next;
    }

    return 0;
}

phys_addr_t pmem_alloc(size_t length, off_t align) {
    phys_addr_t addr = pmem_alloc_helper(length, align, TRUE);

    if(addr != 0)
        return addr;

    return pmem_alloc_helper(length, align, FALSE);
}

void pmem_free(phys_addr_t addr, size_t length) {
    listnode_t* current = reg_list.head;

    while(current) {
        pmem_region_t* reg = (pmem_region_t*)current->payload;

        if(reg_contains(reg, addr)) {
            /* free relative to the current region. */
            addr = PMEM_TO_REGBIT(reg, addr);
            bmap_fill(reg->bmap, 0, addr, addr + PMEM_PAGES(length));
            return;
        }

        current = current->next;
    }

    /* TODO: warning? */
}

