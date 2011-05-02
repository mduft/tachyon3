/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "thread.h"
#include "cpu.h"
#include <process.h>
#include <kheap.h>
#include <spl.h>
#include <mem.h>
#include <x86/gdt.h>

#include "intr.h"
#include <log.h>

#define THR_STACKSIZE 0x8000

thread_t* thr_create(process_t* parent, thread_start_t entry) {
    thread_t* thr = kheap_alloc(sizeof(thread_t));

    if(!thr)
        return NULL;

    memset(thr, 0, sizeof(thread_t));

    thr->id = prc_next_tid(parent);
    thr->parent = parent;
    thr->context = kheap_alloc(sizeof(thr_context_t));
    thr->stack_base = heap_alloc(&parent->stack_heap, THR_STACKSIZE);

    memset(thr->context, 0, sizeof(thr_context_t));

    // TODO: error checking
    
    memset(thr->stack_base, 0, THR_STACKSIZE);

    thr->context->state.rip = (uintptr_t)entry;
    thr->context->state.rsp = ((uintptr_t)thr->stack_base) + THR_STACKSIZE - (sizeof(uintptr_t) * 2);

    if(parent->ring == 0) {
        thr->context->state.ss = GDT_KDATA64;
        thr->context->state.cs = GDT_KCODE64;
    } // TODO: else

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

