/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "process.h"

#include "kheap.h"
#include "mem.h"
#include "spc.h"
#include "log.h"
#include "list.h"
#include "spl.h"

process_t tachyon;

process_t* prc_new() {
    process_t* prc = kheap_alloc(sizeof(process_t));

    if(!prc)
        return NULL;

    memset(prc, 0, sizeof(process_t));

    prc->space = spc_new();
    prc->threads = list_new();

    spl_init(&prc->lock);

    return prc;
}

process_t* prc_destroy(process_t* prc) {
    spl_lock(&prc->lock);

    if(list_size(prc->threads) > 0)
        fatal("trying to destroy process with threads!\n");

    if(prc) {
        spc_delete(prc->space);
        list_delete(prc->threads);
        kheap_free(prc);
    }

    spl_unlock(&prc->lock);

    return NULL;
}

tid_t prc_next_tid(process_t* prc) {
    spl_lock(&prc->lock);
    tid_t ret = prc->ctid++;
    spl_unlock(&prc->lock);

    return ret;
}

void prc_add_thread(process_t* prc, thread_t* thr) {
    spl_lock(&prc->lock);
    list_add(prc->threads, thr);
    spl_unlock(&prc->lock);
}

void prc_remove_thread(process_t* prc, thread_t* thr) {
    spl_lock(&prc->lock);
    list_remove(prc->threads, thr);
    spl_unlock(&prc->lock);
}
