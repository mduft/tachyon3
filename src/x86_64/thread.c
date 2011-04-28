/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "thread.h"
#include "cpu.h"
#include <process.h>
#include <kheap.h>
#include <spl.h>
#include <mem.h>

thread_t* thr_create(process_t* parent, thread_start_t entry) {
    thread_t* thr = kheap_alloc(sizeof(thread_t));

    if(!thr)
        return NULL;

    memset(thr, 0, sizeof(thread_t));

    thr->id = prc_next_tid(parent);
    thr->parent = parent;

    // TODO: create a stack!
    thr->cpu.rip = (uintptr_t)entry;

    return thr;
}

thread_t* thr_delete(thread_t* thr) {
    if(thr) {
        // TODO: destroy stack!
        kheap_free(thr);
    }

    return NULL;
}

thread_t* thr_switch(thread_t* target) {
    cpu_context_t* context = x86_64_ctx_get();

    spl_lock(&context->lock);

    thread_t* old = context->thread;
    context->thread = target;

    /*
     * TODO: _NOT_ in here. save state when entering kernel,
     * restore when leaving it.
    cpu_state_save(&old->cpu);
    cpu_state_restore(&target->cpu);
     */

    spl_unlock(&context->lock);

    return old;
}
