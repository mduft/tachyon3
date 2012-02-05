/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "uapi.h"

static uapi_desc_t const uapi_desc = {
    .syscall=uapi_sysc_call
};

uintptr_t uapi_sysc_call(syscall_t call, uintptr_t p0, uintptr_t p1) {
    uintptr_t res;

    // TODO: sysenter, etc.?
    
    asm volatile(
        "\tint %1\n"
        "\tmov %%rax, %0\n"
        : "=a"(res) 
        : "i"(SYSC_INTERRUPT), "D"(call), "S"(p0), "d"(p1));

    return res;
}

void uapi_thr_trampoline(thread_t* thread, thread_start_t entry) {
    entry(&uapi_desc);

    thread->state = Exited;
    sysc_call(SysSchedule, 0, 0);

    /* never reached - as the thread is aborting, it will never be re-scheduled */
}

///// TEST /////

void test_thr2(uapi_desc_t const* uapi) {
    /// HELP i cannot output anything here!
    return;
}
