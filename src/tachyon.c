/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "tachyon.h"
#include "pmem.h"
#include "log.h"
#include "mboot.h"
#include "spl.h"
#include "rm.h"
#include "extp.h"
#include "mem.h"
#include "spc.h"
#include "process.h"
#include "ksym.h"
#include "intr.h"
#include "kheap.h"
#include "sched.h"
#include "syscall.h"

/**
 * the initial state at boot. contains various boot relevant data,
 * saved by the early entry routine
 */
init_state_t const boot_state;

// -- TESTTEST
void test_thr() {
    int level = 0;
    thread_t* thr = thr_current();

    while(true) {
        for(int i = 0; i < 0x8FFFFF; ++i);

        info("%d: hello thread %d\n", thr->id, level++);
    }
}
// -- TESTTEST

void init_subsys(char const* tag, extp_func_t cb, char const* descr) {
    info("initializing %s\n", descr);
    cb();
}

void boot() {
    log_init();

    /* initialize early kernel extensions (before memory is initialized!) */
    extp_iterate(EXTP_EARLY_KINIT, init_subsys);

    info("tachyon 3.0 built " __TIMESTAMP__ "\n");

    pmem_init();

    /* initialize the BSP. this creates the initial cpu context and state */
    extp_iterate(EXTP_CPUINIT, init_subsys);

    /* initialize the core process with the current address
     * space, and other relevant data. */
    process_t** pcore = (process_t**)&core;
    *pcore = prc_new(spc_current(), PRIO_NORMAL, RING_KERNEL);

    if(!core)
        fatal("failed to create core process\n");

    /* initial thread. it is marked as exited, as this should
     * never return here. */
    thread_t* init = thr_create(core, NULL);
    init->state = Exited;
    thr_switch(init);

    /* initialize kernel internals registered as extension
     * points in no specific order */
    extp_iterate(EXTP_PLATFORM_INIT, init_subsys);
    extp_iterate(EXTP_TIMER_INIT, init_subsys);

    info("kernel heap: used bytes: %d, allocated blocks: %d\n", kheap.state.used_bytes, kheap.state.block_count);

    // -- TESTTEST
    thread_t* thr1 = thr_create(core, test_thr);
    sched_add(thr1);

    thread_t* thr2 = thr_create(core, test_thr);
    sched_add(thr2);
    // -- TESTTEST
    
    // and start the scheduler.
    sched_start();

    fatal("kernel ended unexpectedly.\n");
}
