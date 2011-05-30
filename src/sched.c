/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "sched.h"
#include "log.h"
#include "extp.h"
#include "list.h"
#include "thread.h"
#include "spl.h"

list_t* _sched_queue = NULL;
spinlock_t _sched_lock;

static void sched_init_ext(char const* tag, extp_func_t cb, char const* descr) {
    trace("initializing scheduler dependant: %s\n", descr);
    cb();
}

static void sched_init() {
    _sched_queue = list_new();
    spl_init(&_sched_lock);

    // now initialize all things that depend on the scheduler being present
    extp_iterate(EXTP_SCHEDINIT, sched_init_ext);
}

INSTALL_EXTENSION(EXTP_KINIT, sched_init, "simple non-priority scheduler");

void sched_schedule() {
    // find a thread to schedule. the scheduled thread is removed from the
    // queue, and the currently-run thread is re-added to the queue.
    thread_t* old = thr_current();

    // TODO: check timeslice!

    spl_lock(&_sched_lock);

    list_node_t* node = list_begin(_sched_queue);

    while(node) {
        thread_t* thr = (thread_t*)node->data;

        if(thr && thr->state == Runnable) {
            thr_switch(thr);

            list_remove(_sched_queue, thr);
            list_add(_sched_queue, old);

            goto done;
        }

        node = node->next;
    }

    fatal("no thread left to schedule - this is bad!\n");

  done:
    spl_unlock(&_sched_lock);
}

void sched_yield() {
    NOT_IMPLEMENTED(__func__)
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

