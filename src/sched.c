/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "sched.h"
#include "log.h"
#include "extp.h"
#include "list.h"
#include "thread.h"
#include "spl.h"
#include "syscall.h"
#include "intr.h"
#include "tmr.h"
#include "process.h"

static list_t* _sched_queue = NULL;
static spinlock_t _sched_lock;

static void sched_init_ext(char const* tag, extp_func_t cb, char const* descr) {
    info("initializing scheduler dependant: %s\n", descr);
    cb();
}

static void sched_init() {
    _sched_queue = list_new();
    spl_init(&_sched_lock);

    // now initialize all things that depend on the scheduler being present
    extp_iterate(EXTP_SCHEDINIT, sched_init_ext);
}

void sched_start() {
    // intialize heartbeat timer.
    tmr_schedule(sched_schedule, SCHED_TIMESLICE, false);

    info("waiting for scheduler to take over ...\n");

    // enable hardware interrupts, so the timesource fires.
    intr_enable();

    // wait for the timer to take over. this thread is now stopped.
    asm volatile("1: hlt; jmp 1b;");
}

INSTALL_EXTENSION(EXTP_PLATFORM_INIT, sched_init, "simple scheduler");

void sched_schedule() {
    // find a thread to schedule. the scheduled thread is removed from the
    // queue, and the currently-run thread is re-added to the queue.
    spl_lock(&_sched_lock);

    thread_t* old = thr_current();

    if(old) {
        switch(old->state) {
        case Runnable:
            // TODO: check timeslice, and maybe not switch? timers are
            // set up to fire at timeslice boundaries anyway...
            break;
        case Yielded:
            old->state = Runnable;
            break;
        default:
            // not relevant here.
            break;
        }
    }

    list_node_t* node = list_begin(_sched_queue);

    // TODO: check priorities!
    while(node) {
        thread_t* thr = (thread_t*)node->data;

        if(thr && thr->state == Runnable) {
            trace("chosen thread: %p in process %d\n", thr->id, thr->parent->id);

            thr_switch(thr);

            list_remove(_sched_queue, thr);

            if(old)
                list_add(_sched_queue, old);

            goto done;
        }

        node = node->next;
    }

    // let things stay as they are if only one thread exists.
    if(old->state == Runnable)
        goto done;

    fatal("no thread left to schedule - this is bad!\n");

  done:
    spl_unlock(&_sched_lock);
}

void sched_yield() {
    thread_t * thr = thr_current();
    thr->state = Yielded;

    sysc_call(SysSchedule, 0, 0);

    return;
}

void sched_add(thread_t* thread) {
    spl_lock(&_sched_lock);
    list_add(_sched_queue, thread);
    spl_unlock(&_sched_lock);
}

void sched_remove(thread_t* thread) {
    spl_lock(&_sched_lock);
    list_remove(_sched_queue, thread);
    spl_unlock(&_sched_lock);
}

void sched_wait(thread_t* thread, uintptr_t oid) {
    NOT_IMPLEMENTED(__func__)
}

void sched_wake(uintptr_t oid) {
    NOT_IMPLEMENTED(__func__)
}

