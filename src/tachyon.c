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
#include "uapi.h"

#include "vmem_mgmt.h"

/**
 * the initial state at boot. contains various boot relevant data,
 * saved by the early entry routine
 */
init_state_t const boot_state;

// -- TESTTEST
void test_thr(uapi_desc_t const* uapi) {
    thread_t* thr = thr_current();

    uint64_t start = systime();
    uint64_t stopAfter = 10 * 1000 * 1000;

    while(systime() <= start + stopAfter) {
        uintptr_t x;
        asm volatile("mov %%rsp, %0" : "=m"(x));
        info("%d: hello thread %ld (%p)\n", thr->id, systime(), x);
        systime_stall(10000);
    }
    info("quitting...\n");
}

///// TEST /////

void SECTION(SECT_USER_CODE) test_thr2(uapi_desc_t const* uapi) {
    for(int i = 0; i < 1000; ++i) {
        uapi->syscall(SysLog, Info, (uintptr_t)"hello userspace\n");
    }
    uapi->syscall(SysLog, Info, (uintptr_t)"quitting...\n");
    uapi->syscall(SysThrExit, 0, 0);
    uapi->syscall(SysLog, Error, (uintptr_t)"still here?!\n");
    return;
}
// -- TESTTEST

void init_subsys(char const* tag, extp_func_t cb, char const* descr) {
    info("initializing %s\n", descr);
    cb();
}

/**
 * Contains the local data required during the boot() function.
 * It is required to have that information in a global scope, since
 * stacks are swapped in between which would doom that informations.
 */
static struct {
    thread_t* init;
    rd_header_t* rd;
    pmem_info_t pmem;
} locals;

void boot() {
    /* initialize interrupt dispatcher tables as early as possible. */
    intr_init();

    /* setup system call interrupt modes */
    sysc_init();

    /* initialize the serial log early on boot, so we have debug output from
     * nearly the start on */
    serial_log_init();
    serial_port_init(PORT_COM2);

    /* initialize generic pagefault handler */
    pgflt_init();

    /* initialize dynamic GDT lock */
    dyngdt_init_spinlock();

    /* initialize the physical memory. */
    pmem_init();

    /* find and protect initial ram disc */
    locals.rd = mboot_find_rd();

    /* initialize the userspace api mappings */
    uapi_init();

    /* initialize logging on the screen */
    cga_init();

    info("tachyon 3.0 built " __TIMESTAMP__ "\n");

    /* initialize the BSP. this creates the initial cpu context and state */
    cpu_bsp_init();

    /* dump IDT information */
    intr_dump_idt();

    /* register system calls */
    log_init();
    thr_init();
    sched_init();

    /* initialize the core process with the current address
     * space, and other relevant data. */
    process_t** pcore = (process_t**)&core;
    *pcore = prc_new(spc_current(), PrioKernel);

    if(!core)
        fatal("failed to create core process\n");

    /* initial thread. it is marked as exited (below), as this should
     * never return here. */
    locals.init = thr_create(core, NULL, IsolationKernel);
    locals.init->state = Runnable;
    thr_switch(locals.init);

    /* need to set up a save stack for the kernel level. */
    asm volatile (
        "movq %0,%%rsp; movq %0, %%rbp; pushq $0; pushq $0" :: "d"(locals.init->stack->top - (sizeof(uintptr_t) * 2)) : "rsp"
    );

    /* initialize some more */
    ioapic_init();
    rm_init();

    /* initialize the kernels system timer. this may rely on
     * platform components (fex. i/o apic)! */
    systime_init();

    /* initialize timer related subsystems. those may rely on
     * the kernel system timesource beeing initialized */
    tmr_init();

    info("kernel heap: used bytes: %ld, allocated blocks: %ld\n", kheap.state.used_bytes, kheap.state.block_count);
    locals.pmem = pmem_info();
    info("physical mem: used pages: %ld (%ld kb), free pages: %ld (%ld kb)\n",
        locals.pmem.alloc_pages, (locals.pmem.alloc_pages * PMEM_PAGESIZE) / 1024,
        locals.pmem.free_pages, (locals.pmem.free_pages * PMEM_PAGESIZE) / 1024);

    idle_init();

    // TODO: kick off initial threads.
    // FIXME: local variables are BAD in this method - this only works "by accident"
    // TEST
    for(size_t i = 0; i < 2; ++i) {
        thread_t* thr = thr_create(core, test_thr, IsolationKernel);
        sched_add(thr);
    }

    // a user space process... :)
    process_t* uproc = prc_new(spc_new(), PrioNormal);
    thread_t* thr = thr_create(uproc, test_thr2, IsolationUser);
    thr->priority=PrioKernel; // tmp - scheduler only chooses kernel currently
    sched_add(thr);

    // TEST
    info("ramdisk at: %p\n", locals.rd);

    // and start the scheduler.
    locals.init->state = Exited;
    sched_start();

    fatal("kernel ended unexpectedly.\n");
}
