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
#include "systime.h"
#include "serial.h"
#include "dyngdt.h"
#include "cga.h"
#include "cpu.h"
#include "pgflt.h"
#include "ioapic.h"
#include "tmr.h"
#include "idle.h"

/**
 * the initial state at boot. contains various boot relevant data,
 * saved by the early entry routine
 */
init_state_t const boot_state;

// -- TESTTEST
void test_thr() {
    thread_t* thr = thr_current();

    while(true) {
        info("%d: hello thread %ld\n", thr->id, systime());
    }
}
// -- TESTTEST

void init_subsys(char const* tag, extp_func_t cb, char const* descr) {
    info("initializing %s\n", descr);
    cb();
}

void boot() {
    /* basic logging system initialization, there are no writers initialized
     * however (serial and cga will follow a little later). */
    log_init();

    /* initialize the serial log early on boot, so we have debug output from
     * nearly the start on */
    serial_log_init();
    serial_port_init(PORT_COM2);

    /* initialize generic pagefault handler */
    pgflt_init();

    /* initialize dynamic GDT lock */
    dyngdt_init_spinlock();

    /* initialize the physical memory */
    pmem_init();

    /* initialize logging on the screen */
    cga_init();

    info("tachyon 3.0 built " __TIMESTAMP__ "\n");

    /* initialize the BSP. this creates the initial cpu context and state */
    cpu_bsp_init();

    /* initialize the core process with the current address
     * space, and other relevant data. */
    process_t** pcore = (process_t**)&core;
    *pcore = prc_new(spc_current(), Kernel, RING_KERNEL);

    if(!core)
        fatal("failed to create core process\n");

    /* initial thread. it is marked as exited (below), as this should
     * never return here. */
    thread_t* init = thr_create(core, NULL);
    init->state = Runnable;
    thr_switch(init);

    /* initialize some more */
    ioapic_init();
    rm_init();
    sched_init();
    sysc_init();

    /* initialize the kernels system timer. this may rely on
     * platform components (fex. i/o apic)! */
    systime_init();

    /* initialize timer related subsystems. those may rely on
     * the kernel system timesource beeing initialized */
    tmr_init();

    info("kernel heap: used bytes: %d, allocated blocks: %d\n", kheap.state.used_bytes, kheap.state.block_count);
    pmem_info_t info = pmem_info();
    info("physical mem: used pages: %d, free pages: %d\n", info.alloc_pages, info.free_pages);

    idle_init();

    // TODO: kick off initial threads.

    // and start the scheduler.
    init->state = Exited;
    sched_start();

    fatal("kernel ended unexpectedly.\n");
}
