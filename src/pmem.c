/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "pmem.h"
#include "bmap.h"
#include "log.h"
#include "extp.h"
#include "kheap.h"
#include "vmem.h"
#include "spl.h"
#include "ldsym.h"

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
typedef struct __tag_pmem_region {
    bitmap_t* bmap;     /**< the bitmap denoting page level allocation */
    phys_addr_t start;  /**< the lowest page's address managed by this region */
    size_t length;      /**< the length in bytes of this region */
    struct __tag_pmem_region* next; /**< the next region in the list */
} pmem_region_t;

/** This defines the struct to hold the initial memory region */
static pmem_region_t first_region;

/** This is the head of the region list */
static pmem_region_t* pmem_region_head;

/** This is the tail of the region list */
static pmem_region_t* pmem_region_tail;

/** The global lock across all cpus */
static spinlock_t pmem_lock;

/** 
 * This bitmap holds the individual bits for each page of
 * bootstrap memory 
 */
static bitmap_t first_bmap;

/**
 * This represents the real storage for the bitmap above.
 * it can hold bits for up to 4MB of physical memory, which
 * is the bare minimum that must be available at boot.
 */
static uint32_t first_storage[32];

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
    bmap_init(&first_bmap, first_storage, (sizeof(first_storage) * 8));

    first_region.bmap = &first_bmap;
    first_region.start = 0;
    first_region.length = ((first_bmap.bits - (1 /* savety zone */)) * PMEM_PAGESIZE);
    first_region.next = NULL;

    pmem_region_head = &first_region;
    pmem_region_tail = pmem_region_head;

    spl_init(&pmem_lock);

    /* reserve the page at address zero, as most people don't
     * know how to handle it. rm_init() needs to spare this
     * page when reserving real mode memory! */
    pmem_reserve(0, PMEM_PAGESIZE);

    /* from 4K to 0xa0000 (start of kernel memory) for
     * real mode */
    pmem_reserve(PMEM_PAGESIZE, PMEM_PAGESIZE * 158);

    /* reserve the kernel's physical memory, so nobody
     * else tries to use it */
    if(!pmem_reserve(0xA0000, (((size_t)&_core_lma_ebss) - 0xA0000))) {
        error("failed to protect physical lower and kernel memory\n");
    }

    /* here we are sufficiently initialized, so virtual memory
     * can allocate physical memory for the initial mappings,
     * required to get the kernel heap working. this in turn
     * enables us, to allocate more memory to add additional
     * regions to the physical memory. */
    kheap_init();

    extp_iterate(EXTP_PMEM_REGION, pmem_iterate_extp);

    /* some debugging information */
    pmem_region_t* current = pmem_region_head;
    while(current) {
        trace("pmem: %p - %p (%dKB)\n", current->start, 
            current->start + current->length, current->length / 1024);
        current = current->next;
    }
}

void pmem_add(phys_addr_t start, size_t length) {
    pmem_region_t* reg;
    
recheck:

    reg = pmem_region_head;
    while(reg) {
        if(start >= reg->start && start < (reg->start + reg->length)) {
            /* start within this region */
            if((start + length) <= (reg->start + reg->length)) {
                /* end withing this section, so completely with us already */
                return;
            }

            length -= (reg->start + reg->length) - start;
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

        reg = reg->next;
    }

    /* we have a checked region here, with correct start and end.
     * now allocate the required management structures, etc. */
    pmem_region_t* region = (pmem_region_t*)kheap_alloc(sizeof(pmem_region_t));
    bitmap_t* bmap = bmap_new(PMEM_PAGES(length));

    if(!region || !bmap) {
        error("not enough memory to allocate memory management structures!\n");

        if(region)  kheap_free(region);
        if(bmap)    kheap_free(bmap);

        return;
    }

    region->start = start;
    region->length = length;
    region->bmap = bmap;
    region->next = NULL;

    pmem_region_tail->next = region;
    pmem_region_tail = region;
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
    pmem_region_t* reg = pmem_region_head;

    while(reg) {
        size_t fdeg = (use_fdeg ? bmap_fdeg(reg->bmap) : 0);

        /* make sure there is enough free room in the region to
         * fit the block to be allocated. */
        
        if(fdeg == 0 || ((reg->length * (100-fdeg))) > (length * 100)) {
            size_t idx;

            spl_lock(&pmem_lock);
            
            if(bmap_search(reg->bmap, &idx, 0, PMEM_PAGES(length), (align / PMEM_PAGESIZE), BMAP_SRCH_HINTED)) {
                if(bmap_fill(reg->bmap, 1, idx, idx + PMEM_PAGES(length))) {
                    *addr = PMEM_FROM_REGBIT(reg, idx);
                    spl_unlock(&pmem_lock);
                    return true;
                }
            }

            spl_unlock(&pmem_lock);
        }

        reg = reg->next;
    }

    return false;
}

phys_addr_t pmem_alloc(size_t length, off_t align) {
    phys_addr_t addr;

    if(pmem_alloc_helper(&addr, length, align, true))
        goto ok;

    if(pmem_alloc_helper(&addr, length, align, false))
        goto ok;

    fatal("out of physical memory\n");

  ok:
    return addr;
}

bool pmem_reserve(phys_addr_t addr, size_t length) {
    register phys_addr_t top, cur;
    register bool checkPass = true;

    if(ALIGN_RST(addr, PMEM_PAGESIZE) != 0) {
        fatal("misaligned physical address!\n");
    }

    debug("try to reserve physical memory at %p (length: %d bytes)\n", addr, length);

    spl_lock(&pmem_lock);

next_pass:
    top = (addr + length);
    cur = addr;

    while(top > cur) {
        pmem_region_t* reg = pmem_region_head;

        while(reg) {
            while(pmem_reg_contains(reg, cur)) {
                register size_t idx = PMEM_TO_REGBIT(reg, cur);

                if(checkPass) {
                    if(bmap_get(reg->bmap, idx)) {
                        spl_unlock(&pmem_lock);
                        debug("failed to reserve region, %p already reserved\n", cur);
                        return false;
                    }
                } else {
                    bmap_set(reg->bmap, idx, 1);
                }
                
                cur += PMEM_PAGESIZE;

                if(top <= cur)
                    goto pass_ok;
            }

            reg = reg->next;
        }

        cur += PMEM_PAGESIZE;
    }

pass_ok:
    if(checkPass) {
        checkPass = false;
        goto next_pass;
    }

    spl_unlock(&pmem_lock);

    return true;
}

void pmem_free(phys_addr_t addr, size_t length) {
    pmem_region_t* reg = pmem_region_head;

    while(reg) {
        if(pmem_reg_contains(reg, addr)) {
            /* free relative to the current region. */
            addr = PMEM_TO_REGBIT(reg, addr);
            bmap_fill(reg->bmap, 0, addr, addr + PMEM_PAGES(length));
            return;
        }

        reg = reg->next;
    }

    warn("cannot find physical region to free %p\n", addr);
}

pmem_info_t pmem_info() {
    pmem_region_t* reg = pmem_region_head;
    pmem_info_t info = { 0, 0 };

    while(reg) {
        info.alloc_pages += reg->bmap->set_cnt;
        info.free_pages += reg->bmap->bits - reg->bmap->set_cnt;

        reg = reg->next;
    }

    return info;
}
