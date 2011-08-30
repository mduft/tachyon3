/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "extp.h"
#include "sched.h"
#include "thread.h"
#include "process.h"
#include "log.h"

#define IDLE_LOOPCNT    0xFFFF

static void idle_thread() {
    while(true) {

        info("idling ...\n");

        // endless... :)
        // TODO: better short delay?
        for(int i = 0; i < IDLE_LOOPCNT; ++i) ;
        sched_yield();

    }
}

void idle_init() {
    thread_t* idle_thr = thr_create(core, idle_thread);

    idle_thr->priority = Idle;

    sched_add(idle_thr);
}

