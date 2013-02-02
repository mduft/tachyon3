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

#include <paging.h>

/**
 * The kernel process.
 */
process_t* const core;

process_t* prc_new(spc_t space, priority_t priority, uint8_t ring) {
    process_t* prc = kheap_alloc(sizeof(process_t));

    if(!prc) {
        goto fail;
    }

    memset(prc, 0, sizeof(process_t));

    prc->space = space;

    if(!prc->space) {
        goto fail;
    }

    prc->threads = list_new();

    if(!prc->threads) {
        goto fail;
    }

    // ATTENTION: a rather lengthy operation (but should be ok for
    // process creation): To be able to initialize the heap, the
    // address space of the process must be activated. It is then
    // deactivated again.
    spc_t old = spc_current();
    spc_switch(prc->space);

    prc->heap.start = PHEAP_START;
    prc->heap.end = PHEAP_END;
    prc->heap.space = prc->space;
    // TODO: see above - swap to correct location.
    prc->heap.pg_fl = (PG_WRITABLE | PG_USER);
    
    if(!heap_init(&prc->heap)) {
        goto fail;
    }

    if(ring == RING_KERNEL) {
        prc->stka = kstack_allocator;
    } else {
        stack_allocator_desc_t desc = {
            .top = SHEAP_END,
            .bottom = SHEAP_START,
            .space = prc->space,
            // TODO: see above - swap to correct location.
            // TODO: no execute on stack...?
            .pg_fl = (PG_WRITABLE | PG_USER),
            .fixed = false,
            .global = false
        };

        prc->stka = stka_new(&desc);
    }

    if(!prc->stka) {
        goto fail;
    }

    prc->priority = priority;
    prc->ring = ring;

    // switch back too the previous address space.
    spc_switch(old);

    spl_init(&prc->lock);

    return prc;

fail:
    if(prc) {
        if(prc->stka)
            stka_delete(prc->stka);
        if(prc->heap.state.valid)
            heap_delete(&prc->heap);
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

    stka_delete(prc->stka);
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
