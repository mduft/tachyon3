/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "thread.h"
#include "cpu.h"
#include "gdt.h"
#include "reg.h"
#include <process.h>
#include <kheap.h>
#include <spl.h>
#include <mem.h>
#include <sched.h>
#include <syscall.h>
#include <ksym.h>
#include <spc.h>

#include "intr.h"
#include <log.h>
#include "uapi.h"
#include "syscall.h"

static bool thr_syscall_handler(interrupt_t* state) {
    switch(sysc_get_call(state)) {
    case SysThrExit:
        sysc_call(state, (syscall_handler_t)thr_exit);
        return true;
    default:
        return false;
    }
}

void thr_init() {
    intr_add(SYSC_INTERRUPT, thr_syscall_handler);
}

thread_t* thr_create(process_t* parent, thread_start_t entry, thread_isolation_t isolation) {
    thread_t* thr = kheap_alloc(sizeof(thread_t));

    if(!thr)
        return NULL;

    memset(thr, 0, sizeof(thread_t));

    thr->id = prc_next_tid(parent);
    thr->parent = parent;
    thr->context = kheap_alloc(sizeof(thr_context_t));

    if(isolation == IsolationKernel) {
        thr->stka = kstack_allocator;
    } else {
        thr->stka = parent->stka;
    }

    thr->stack = stka_alloc(thr->stka);
    thr->priority = parent->priority;

    memset(thr->context, 0, sizeof(thr_context_t));

    // TODO: error checking

    thr->context->state.rip = (uintptr_t)uapi_thr_trampoline;
    thr->context->state.rdi = (uintptr_t)thr;
    thr->context->state.rsi = (uintptr_t)entry;
    thr->context->state.rflags = FL_IF; // enable interrupts when starting thread.
    thr->context->state.rsp = thr->stack->top - (sizeof(uintptr_t) * 2);
    thr->context->thread = thr;

    if(isolation == IsolationKernel) {
        thr->context->state.ss = GDT_KDATA64;
        thr->context->state.cs = GDT_KCODE64;
    } else {
        thr->context->state.ss = GDT_UDATA64 | RING_USERSPACE;
        thr->context->state.cs = GDT_UCODE64 | RING_USERSPACE;
    }

    thr->state = Runnable;

    return thr;
}

thread_t* thr_delete(thread_t* thr) {
    if(thr->context) {
        kheap_free(thr->context);
    }
    if(thr->stack) {
        stka_free(thr->stka, thr->stack);
    }
    if(thr) {
        kheap_free(thr);
    }

    return NULL;
}

thread_t* thr_switch(thread_t* target) {
    thr_context_t* old = thr_ctx_get();
    thr_ctx_set(target->context);

    if(old->thread == NULL) {
        // this is a dummy context, as the cpu context may never
        // be NULL. This is only created when initializing a CPU.

        // to avoid having to check whether there was a running
        // thread, return the new thread as old one. This saves
        // the caller from having to check NULLs.
        return target;
    }

    if(old->thread->parent != target->parent) {
        spc_switch(target->parent->space);
    }

    return old->thread;
}

thread_t* thr_current() {
    thr_context_t* ctx = thr_ctx_get();

    return ctx->thread;
}

void thr_abort(thread_t* target) {
    target->state = Aborting;

    error("thread %d in process %d aborted!\n", target->id, target->parent->id);

    list_t* trace = ksym_trace();
    ksym_write_trace(Error, trace);
    ksym_delete(trace);

    if(target->parent->id == 0) {
        // uh oh - kernel thread crashed....!!
        abort();
    }

    sched_schedule();
}

void thr_exit(thread_t* thread) {
    trace("thread %d in process %d exiting normally\n", thread->id, thread->parent->id);
    thread->state = Exited;

    sched_schedule();
}
