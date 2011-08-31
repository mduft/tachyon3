/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "extp.h"
#include "sched.h"
#include "thread.h"
#include "process.h"
#include "log.h"

static void idle_thread() {
    while(true) {
        trace("idling ...\n");
        asm volatile("hlt");
    }
}

void idle_init() {
    thread_t* idle_thr = thr_create(core, idle_thread);

    idle_thr->priority = Idle;

    sched_add(idle_thr);
}

