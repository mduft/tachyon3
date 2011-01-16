/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "pmem.h"
#include "bmap.h"
#include "list.h"
#include "log.h"
#include "extp.h"
#include "kheap.h"
#include "vmem.h"

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
 * it can hold bits for up to 640KB of physical memory, which
 * is the bare minimum that must be available at boot.
 */
static uint32_t first_storage[5];

/**
 * Checks whether a given adress lies within a region.
 *
 * @param reg   the region to check.
 * @param addr  the address to check.
 */
static inline bool pmem_reg_contains(pmem_region_t* reg, phys_addr_t addr) {
    return (addr >= reg->start && addr < (reg->start + reg->length));
}

/**
 * Used to iterate over all extension points for the
 * physical memory regions. Each of those callbacks
 * can the in turn call pmem_add() to add more physical
 * memory regions.
 *
 * @param tag   always "pmem.region"
 * @param cb    the callback
 * @param desc  ignored
 */
static void pmem_iterate_extp(char const* tag, extp_func_t cb, char const* desc) {
    if(cb) cb();
}

void pmem_init() {
    reg_list.head = &first_node;
    reg_list.head->next = NULL;
    reg_list.head->payload = (uintptr_t)&first_region;

    bmap_init(&first_bmap, first_storage, (sizeof(first_storage) * 8));

    first_region.bmap = &first_bmap;
    first_region.start = 0;
    first_region.length = ((first_bmap.bits - (1 /* savety zone */)) * PMEM_PAGESIZE);

    /* immediately reserve the first physical page (real
     * mode IVT). */
    if(!pmem_reserve(0x0, 0x1000)) {
        warn("failed protecting real mode IVT.\n");
    }

    /* initialize virtual memory. this releases the identity
     * mapping for the physical kernel load address */
    vmem_init();

    /* here we are sufficiently initialized, so virtual memory
     * can allocate physical memory for the initial mappings,
     * required to get the kernel heap working. this in turn
     * enables us, to allocate more memory to add additional
     * regions to the physical memory. */
    kheap_init();

    extp_iterate(EXTP_PMEM_REGION, pmem_iterate_extp);

    /* some debugging information */
    listnode_t* current = reg_list.head;
    while(current) {
        pmem_region_t* reg = (pmem_region_t*)current->payload;
        trace("pmem: %p - %p (%dKB)\n", reg->start, 
            reg->start + reg->length, reg->length / 1024);
        current = current->next;
    }
}

void pmem_add(phys_addr_t start, size_t length) {
    listnode_t* current;
    
recheck:

    current = reg_list.head;
    while(current) {
        pmem_region_t* reg = (pmem_region_t*)current->payload;

        if(start >= reg->start && start < (reg->start + reg->length)) {
            /* start within this region */
            if((start + length) <= (reg->start + reg->length)) {
                /* end withing this section, so completely with us already */
                return;
            }

            start = reg->start + reg->length;
            goto recheck;
        }

        if((start + length) > reg->start && 
            (start + length) < (reg->start + reg->length)) 
        {
            /* end within this region */
            if(start >= reg->start) {
                /* start too, so no more left over. */
                return;
            }

            length = (reg->start - start);
            goto recheck;
        }

        current = current->next;
    }

    /* we have a checked region here, with correct start and end.
     * now allocate the required management structures, etc. */
    pmem_region_t* region = (pmem_region_t*)kheap_alloc(sizeof(pmem_region_t));
    listnode_t* node = (listnode_t*)kheap_alloc(sizeof(listnode_t));
    bitmap_t* bmap = bmap_new(PMEM_PAGES(length));

    if(!region || !node || !bmap) {
        error("not enough memory to allocate memory management structures!\n");

        if(region)  kheap_free(region);
        if(node)    kheap_free(node);
        if(bmap)    kheap_free(bmap);

        return;
    }

    region->start = start;
    region->length = length;
    region->bmap = bmap;

    node->payload = (uintptr_t)region;
    node->next = reg_list.head;
    reg_list.head = node;
}

/**
 * Helper to really allocate memory either with filling degree
 * hinting on or off. only used from the real pmem_alloc.
 *
 * @param[out] addr the allocated address if successfull
 * @param length    the block length to allocate
 * @param align     the minimum alignment for the block.
 * @param use_fdeg  whether to take filling degree into
 *                  account to speed up free region search.
 * @return          true on success, false on failure.
 */
static bool pmem_alloc_helper(phys_addr_t* addr, size_t length, off_t align, bool use_fdeg) {
    listnode_t* current = reg_list.head;

    while(current) {
        pmem_region_t* reg = (pmem_region_t*)current->payload;
        size_t fdeg = (use_fdeg ? bmap_fdeg(reg->bmap) : 0);

        /* make sure there is enough free room in the region to
         * fit the block to be allocated. */
        
        if(fdeg == 0 || ((reg->length * (100-fdeg))) > (length * 100)) {
            size_t idx;

            /* TODO: lock this allocation. permormance consideration:
             * should we lock the whole loop, or is locking/unlocking
             * fast enough? */

            if(bmap_search(reg->bmap, &idx, 0, PMEM_PAGES(length), (align / PMEM_PAGESIZE), BMAP_SRCH_HINTED)) {
                if(bmap_fill(reg->bmap, 1, idx, idx + PMEM_PAGES(length))) {
                    *addr = PMEM_FROM_REGBIT(reg, idx);
                    return true;
                }
            }

            /* TODO: unlock */
        }

        current = current->next;
    }

    return false;
}

phys_addr_t pmem_alloc(size_t length, off_t align) {
    phys_addr_t addr;
    
    if(pmem_alloc_helper(&addr, length, align, true))
        return addr;

    if(pmem_alloc_helper(&addr, length, align, false))
        return addr;

    fatal("out of physical memory\n");
}

bool pmem_reserve(phys_addr_t addr, size_t length) {
    register phys_addr_t top, cur;
    register bool checkPass = true;

    if(ALIGN_RST(addr, PMEM_PAGESIZE) != 0) {
        fatal("misaligned physical address!\n");
    }

    /* TODO: lock this. this should be used during initialization only,
     * so it is not so important for this code to be ultra-performant. */

next_pass:
    top = (addr + length);
    cur = addr;

    while(top > cur) {
        listnode_t* current = reg_list.head;

        while(current) {
            pmem_region_t* reg = (pmem_region_t*)current->payload;

            while(pmem_reg_contains(reg, cur)) {
                register size_t idx = PMEM_TO_REGBIT(reg, cur);

                if(checkPass) {
                    if(bmap_get(reg->bmap, idx)) {
                        return false;
                    }
                } else {
                    bmap_set(reg->bmap, idx, 1);
                }
                
                cur += PMEM_PAGESIZE;

                if(top <= cur)
                    goto pass_ok;
            }

            current = current->next;
        }

        cur += PMEM_PAGESIZE;
    }

pass_ok:
    if(checkPass) {
        checkPass = false;
        goto next_pass;
    }

    return true;
}

void pmem_free(phys_addr_t addr, size_t length) {
    listnode_t* current = reg_list.head;

    while(current) {
        pmem_region_t* reg = (pmem_region_t*)current->payload;

        if(pmem_reg_contains(reg, addr)) {
            /* free relative to the current region. */
            addr = PMEM_TO_REGBIT(reg, addr);
            bmap_fill(reg->bmap, 0, addr, addr + PMEM_PAGES(length));
            return;
        }

        current = current->next;
    }

    warn("cannot find physical region to free %p\n", addr);
}

