/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "thread.h"
#include "cpu.h"
#include <process.h>
#include <kheap.h>
#include <spl.h>
#include <mem.h>

#include "intr.h"
#include <log.h>

thread_t* thr_create(process_t* parent, thread_start_t entry) {
    thread_t* thr = kheap_alloc(sizeof(thread_t));

    if(!thr)
        return NULL;

    memset(thr, 0, sizeof(thread_t));

    thr->id = prc_next_tid(parent);
    thr->parent = parent;
    thr->context = kheap_alloc(sizeof(thr_context_t));

    // TODO: create a stack!
    thr->context->state.rip = (uintptr_t)entry;

    return thr;
}

thread_t* thr_delete(thread_t* thr) {
    if(thr->context) {
        kheap_free(thr->context);
    }
    // TODO: destroy stack!
    if(thr) {
        kheap_free(thr);
    }

    return NULL;
}

thread_t* thr_switch(thread_t* target) {
    thr_context_t* old = x86_64_ctx_get();
    x86_64_ctx_set(target->context);

    if(old->thread == NULL) {
        // this is a dummy context, as the cpu context may never
        // be NULL. This is only created when initializing a CPU.
        // Since there is always at least the idle thread for the
        // CPU, release it now.
        kheap_free(old);

        // to avoid having to check whether there was a running
        // thread, return the new thread as old one. This saves
        // the caller from having to check NULLs.
        return target;
    }

    return old->thread;
}

