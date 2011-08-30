/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "kheap.h"
#include "mem.h"

#include <spc.h>
#include <log.h>
#include "paging.h"

heap_t kheap;

void kheap_init() {
    memset(&kheap, 0, sizeof(heap_t));

    kheap.start = KHEAP_START;
    kheap.end = KHEAP_END;
    kheap.space = spc_current();
    kheap.pg_fl = (PG_GLOBAL | PG_WRITABLE);

    if(!heap_init(&kheap))
        fatal("cannot initialize kernel heap!\n");
}

