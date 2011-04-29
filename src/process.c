/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "process.h"

#include "kheap.h"
#include "mem.h"
#include "spc.h"
#include "log.h"
#include "list.h"
#include "spl.h"
#include "heap.h"

// TODO: hack: swap platform specific intialization of process to correct location.
#include <x86/paging.h>

process_t* prc_new() {
    process_t* prc = kheap_alloc(sizeof(process_t));

    if(!prc) {
        goto fail;
    }

    memset(prc, 0, sizeof(process_t));

    prc->space = spc_new();

    if(!prc->space) {
        goto fail;
    }

    prc->threads = list_new();

    if(!prc->threads) {
        goto fail;
    }

    prc->heap.start = PHEAP_START;
    prc->heap.end = PHEAP_END;
    prc->heap.space = prc->space;
    // TODO: see above - swap to correct location.
    prc->heap.pg_fl = (PG_WRITABLE | PG_USER);
    
    if(!heap_init(&prc->heap)) {
        goto fail;
    }

    prc->stack_heap.start = SHEAP_START;
    prc->stack_heap.end = SHEAP_END;
    prc->stack_heap.space = prc->space;
    // TODO: see above - swap to correct location.
    // TODO: no execute on stack...?
    prc->stack_heap.pg_fl = (PG_WRITABLE | PG_USER);

    if(!heap_init(&prc->stack_heap)) {
        goto fail;
    }

    spl_init(&prc->lock);

    return prc;

fail:
    if(prc) {
        if(prc->stack_heap.state.valid)
            heap_delete(&prc->stack_heap);
        if(prc->heap.state.valid)
            heap_delete(&prc->heap);
        if(prc->space)
            spc_delete(prc->space);
        if(prc->threads)
            list_delete(prc->threads);
        
        kheap_free(prc);
    }

    error("failed to create process!\n");

    return NULL;
}

process_t* prc_destroy(process_t* prc) {
    if(!prc)
        return NULL;

    spl_lock(&prc->lock);

    if(list_size(prc->threads) > 0)
        fatal("trying to destroy process with threads!\n");

    heap_delete(&prc->stack_heap);
    heap_delete(&prc->heap);

    spc_delete(prc->space);
    list_delete(prc->threads);
    kheap_free(prc);

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
