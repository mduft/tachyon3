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

/**
 * the initial state at boot. contains various boot relevant data,
 * saved by the early entry routine
 */
init_state_t const boot_state;

// -- TESTTEST
#include "intr.h"
#include "x86_64/cpu.h"

void test_thr() {
    thread_t* thr = thr_current();

    while(true) {
        info("%d: (intr: %d) hello thread %ld\n", thr->id, intr_state(), systime());
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
    *pcore = prc_new(spc_current(), Kernel, RING_KERNEL);

    if(!core)
        fatal("failed to create core process\n");

    /* initial thread. it is marked as exited, as this should
     * never return here. */
    thread_t* init = thr_create(core, NULL);
    init->state = Runnable;
    thr_switch(init);

    /* initialize kernel internals registered as extension
     * points in no specific order */
    extp_iterate(EXTP_PLATFORM_INIT, init_subsys);

    /* initialize the kernels system timer. this may rely on
     * platform components (fex. i/o apic)! */
    systime_init();

    /* initialize timer related subsystems. those may rely on
     * the kernel system timesource beeing initialized */
    extp_iterate(EXTP_TIMER_INIT, init_subsys);

    info("kernel heap: used bytes: %d, allocated blocks: %d\n", kheap.state.used_bytes, kheap.state.block_count);

    // -- TESTTEST
    /*
    while(true) {
        // interrupts should be disabled!
        x86_64_cpu_state_t before;
        memset(&before, 0, sizeof(before));
        asm volatile("push $0x10; push $0x010101; pushf; push $0x18; push $0x1234; push $1; push $99; lea %0,%%rax; push %%rax; call x86_64_isr_state_save" :: "m"(before) : "rax");

        uintptr_t bp = before.rbp;
        memset(&before, 0xAB, sizeof(before));
        before.rbp = bp;

        asm volatile("call x86_64_isr_state_restore");

        asm volatile("pop %%rax" ::: "rax");
        asm volatile("pop %%rax" ::: "rax");
        asm volatile("pop %%rax" ::: "rax");
        asm volatile("pop %%rax" ::: "rax");
        asm volatile("pop %%rax" ::: "rax");
        asm volatile("pop %%rax" ::: "rax");
        asm volatile("pop %%rax" ::: "rax");
        asm volatile("pop %%rax" ::: "rax");
    }
    */
    // -- TESTTEST

    // -- TESTTEST
    for(uint32_t i = 0; i < 4; ++i) {
        thread_t* thr = thr_create(core, test_thr);
        sched_add(thr);
    }
    // -- TESTTEST
    
    // and start the scheduler.
    init->state = Exited;
    sched_start();

    fatal("kernel ended unexpectedly.\n");
}
