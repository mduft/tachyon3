/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "thread.h"
#include "cpu.h"
#include <process.h>
#include <kheap.h>
#include <spl.h>
#include <mem.h>
#include <x86/gdt.h>
#include <sched.h>

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
    thr->stack = stka_alloc(parent->stka);
    thr->priority = parent->priority;

    memset(thr->context, 0, sizeof(thr_context_t));

    // TODO: error checking

    // TODO: user mode trampoline!
    thr->context->state.rip = (uintptr_t)thr_trampoline;
    thr->context->state.rdi = (uintptr_t)thr;
    thr->context->state.rsi = (uintptr_t)entry;
    thr->context->state.rsp = thr->stack->top - (sizeof(uintptr_t) * 2);
    thr->context->thread = thr;

    if(parent->ring == 0) {
        thr->context->state.ss = GDT_KDATA64;
        thr->context->state.cs = GDT_KCODE64;
    } // TODO: else

    thr->state = Runnable;

    return thr;
}

thread_t* thr_delete(thread_t* thr) {
    if(thr->context) {
        kheap_free(thr->context);
    }
    if(thr->stack) {
        stka_free(thr->parent->stka, thr->stack);
    }
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

void thr_abort(thread_t* target) {
    target->state = Aborting;

    error("thread %d in process %d aborted!\n", target->id, target->parent->id);
    sched_schedule();

    /* never reached - as the thread is aborting, it will never be re-scheduled */
}

void thr_trampoline(thread_t* thread, thread_start_t entry) {
    entry();

    thread->state = Exited;
    
    trace("thread %d in process %d exited\n", thread->id, thread->parent->id);
    sched_schedule();

    /* never reached - as the thread is aborting, it will never be re-scheduled */
}
