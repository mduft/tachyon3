/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "process.h"

#include "kheap.h"
#include "mem.h"
#include "spc.h"
#include "log.h"
#include "list.h"

process_t tachyon;

process_t* prc_new() {
    process_t* prc = kheap_alloc(sizeof(process_t));

    if(!prc)
        return NULL;

    memset(prc, 0, sizeof(prc));

    prc->space = spc_new();
    prc->threads = list_new();

    return prc;
}

process_t* prc_destroy(process_t* prc) {
    if(prc) {
        spc_delete(prc->space);
        list_delete(prc->threads);
        kheap_free(prc);
    }

    return NULL;
}

