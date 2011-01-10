/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "tachyon.h"
#include "pmem.h"
#include "log.h"

/* testing */
#if 1
#include "bmap.h"
#endif

void boot(/*void* mbd, uint32_t mbm*/) {

    log_init();
    log_set_level("screen", Trace);

    pmem_init();

    /* from here on are tests only */
    debug("testing...\n");

    /* test bitmap */
    #if 0
    {
        bitmap_t bm;
        uintptr_t store[4];
        size_t idx;

        if(!bmap_init(&bm, store, 4 * sizeof(uintptr_t))) {
            debug("error: init\n");
        }

        bmap_clear(&bm, 1);

        if(bmap_search(&bm, &idx, 0, 1, 1, 0))
            debug("error: result unexpected (ok)!\n");


        if(bmap_search(&bm, &idx, 0, 1, 1, BMAP_SRCH_BACKWARD))
            debug("error: back result unexpected (ok)!\n");

        if(!bmap_search(&bm, &idx, 1, 2, 1, BMAP_SRCH_HINTED))
            debug("error: result unexpected (!ok)\n");

        if(!bmap_search(&bm, &idx, 1, 3, 3, BMAP_SRCH_HINTED))
            debug("error: result unexpected (!ok)\n");

        if(bm.hint != 6)
            debug("error: hint has wrong value\n");
    }
    #endif

    /* physical memory testing */
    #if 1
    phys_addr_t a1 = pmem_alloc(0x1000, 0x1000);
    phys_addr_t a2 = pmem_alloc(0x10000, 0x1000);
    phys_addr_t a3 = pmem_alloc(0x1000, 0x1000);
    phys_addr_t a4 = pmem_alloc(0x1000, 0x100000);

    debug("pmem_alloc:\n");
    debug("  0x001000, 0x001000: %p\n", a1);
    debug("  0x010000, 0x001000: %p\n", a2);
    debug("  0x001000, 0x001000: %p\n", a3);
    debug("  0x001000, 0x100000: %p\n", a4);

    pmem_free(a1, 0x1000);
    pmem_free(a2, 0x10000);
    pmem_free(a3, 0x1000);
    pmem_free(a4, 0x1000);
    #endif
}
