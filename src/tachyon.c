/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "tachyon.h"
#include "pmem.h"

/* testing */
#if 1
#include "bmap.h"
#include "cga.h"
#endif

void boot(/*void* mbd, uint32_t mbm*/) {

    pmem_init();

    /* from here on are tests only */
    cga_write("testing...\n");

    /* test bitmap */
    #if 0
    {
        bitmap_t bm;
        uintptr_t store[4];
        size_t idx;

        if(!bmap_init(&bm, store, 4 * sizeof(uintptr_t))) {
            cga_write("error: init\n");
        }

        bmap_clear(&bm, 1);

        if(bmap_search(&bm, &idx, 0, 1, 1, 0))
            cga_write("error: result unexpected (ok)!\n");


        if(bmap_search(&bm, &idx, 0, 1, 1, BMAP_SRCH_BACKWARD))
            cga_write("error: back result unexpected (ok)!\n");

        if(!bmap_search(&bm, &idx, 1, 2, 1, BMAP_SRCH_HINTED))
            cga_write("error: result unexpected (!ok)\n");

        if(!bmap_search(&bm, &idx, 1, 3, 3, BMAP_SRCH_HINTED))
            cga_write("error: result unexpected (!ok)\n");

        if(bm.hint != 6)
            cga_write("error: hint has wrong value\n");
    }
    #endif

    /* physical memory testing */
    #if 1
    #endif
}
