/* Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
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
#include "mem.h"

static list_t* _sched_queues[PrioMaxPrio];
static spinlock_t _sched_lock;

static bool sched_syscall_handler(interrupt_t* state) {
    switch(sysc_get_call(state)) {
    case SysSchedule:
        sysc_call(state, (syscall_handler_t)sched_schedule);
        return true;
    case SysYield:
        sysc_call(state, (syscall_handler_t)sched_yield);
        return true;
    default:
        return false;
    }
}

void sched_init() {
    if(intr_state())
        fatal("interrupts may not be enabled in sched_init()\n");

    spl_init(&_sched_lock);

    memset(_sched_queues, 0, sizeof(_sched_queues));
    intr_add(SYSC_INTERRUPT, sched_syscall_handler);
}

void sched_start() {
    // intialize heartbeat timer.
    tmr_schedule(sched_schedule, SCHED_TIMESLICE_US, false);

    info("waiting for scheduler to take over ...\n");

    intr_enable(true);

    // wait for the timer to take over. this thread is now stopped.
    asm volatile("1: hlt; jmp 1b;");
}

static void sched_add_unlocked(thread_t* thread) {
    if(!_sched_queues[thread->priority])
        _sched_queues[thread->priority] = list_new();

    list_add(_sched_queues[thread->priority], thread);
}

static void sched_remove_unlocked(thread_t* thread) {
    list_remove(_sched_queues[thread->priority], thread);
}

static thread_t* sched_choose() {
    for(priority_t i = PrioKernel; i < PrioMaxPrio; --i) {
        list_node_t* node = list_begin(_sched_queues[i]);

        // TODO: check priorities!
        while(node) {
            thread_t* thr = (thread_t*)node->data;

            if(thr && thr->state == Runnable) {
                return thr;
            }

            node = node->next;
        }
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

    // BUG: this algorithm will start to choose the idle thread when
    // only one other thread is remaining runnable.

    thread_t* thr = sched_choose(old);

    if(thr) {
        trace("switching: %d:%d (%d)\n", thr->parent->id, thr->id, thr->priority);

        thr->preempt_at = systime() + SCHED_TIMESLICE_US;

        thr_switch(thr);
        sched_remove_unlocked(thr);

        if(old)
            sched_add_unlocked(old);

        goto done;
    }

    // let things stay as they are if only one thread exists.
    if(old->state == Runnable) {
        goto done;
    }

    fatal("no thread left to schedule - this is bad!\n");

  done:
    spl_unlock(&_sched_lock);
}

void sched_yield() {
    thread_t * thr = thr_current();
    thr->state = Yielded;

    sched_schedule();
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

