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
#include "systime.h"

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
    tmr_schedule(sched_schedule, SCHED_TIMESLICE_US, false);

    info("waiting for scheduler to take over ...\n");

    intr_enable();

    // wait for the timer to take over. this thread is now stopped.
    asm volatile("1: hlt; jmp 1b;");
}

INSTALL_EXTENSION(EXTP_PLATFORM_INIT, sched_init, "simple scheduler");

static void sched_add_unlocked(thread_t* thread) {
    list_add(_sched_queue, thread);
}

static void sched_remove_unlocked(thread_t* thread) {
    list_remove(_sched_queue, thread);
}

static thread_t* sched_choose() {
     list_node_t* node = list_begin(_sched_queue);

    // TODO: check priorities!
    while(node) {
        thread_t* thr = (thread_t*)node->data;

        if(thr && thr->state == Runnable) {
            return thr;
        }

        node = node->next;
    }

    return NULL;
}

void sched_schedule() {
    // find a thread to schedule. the scheduled thread is removed from the
    // queue, and the currently-run thread is re-added to the queue.
    spl_lock(&_sched_lock);

    thread_t* old = thr_current();

    if(old) {
        switch(old->state) {
        case Runnable:
            // don't stop if the thread still has time!
            if(old->preempt_at > systime()) {
                goto done;
            }
            break;
        case Yielded:
            // timeslice is given up.
            old->state = Runnable;
            break;
        default:
            // not relevant here.
            break;
        }
    }

    thread_t* thr = sched_choose();

    if(thr) {
        thr->preempt_at = systime() + SCHED_TIMESLICE_US;

        thr_switch(thr);
        sched_remove_unlocked(thr);

        if(old)
            sched_add_unlocked(old);

        goto done;
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
    sched_add_unlocked(thread);
    spl_unlock(&_sched_lock);
}

void sched_remove(thread_t* thread) {
    spl_lock(&_sched_lock);
    sched_remove_unlocked(thread);
    spl_unlock(&_sched_lock);
}

void sched_wait(thread_t* thread, uintptr_t oid) {
    NOT_IMPLEMENTED(__func__)
}

void sched_wake(uintptr_t oid) {
    NOT_IMPLEMENTED(__func__)
}

