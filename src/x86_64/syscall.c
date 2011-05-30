/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include <syscall.h>
#include <extp.h>
#include <thread.h>
#include <log.h>
#include <sched.h>

#include "intr.h"

static bool sysc_handler(interrupt_t* state) {
    thread_t* thr = state->ctx->thread;

    if(!thr)
        fatal("no thread associated with execution context!\n");

    thr->syscall++;

    state->ctx->state.rax = sysc_call(state->ctx->state.rdi, 
        state->ctx->state.rsi, state->ctx->state.rdx);

    thr->syscall--;
    return true;
}

static void sysc_init() {
    intr_add(SYSC_INTERRUPT, sysc_handler);
}

INSTALL_EXTENSION(EXTP_KINIT, sysc_init, "system call handler");

bool sysc_active() {
    thread_t* thr = x86_64_ctx_get()->thread;

    if(!thr)
        fatal("no thread associated with execution context!\n");

    return (thr->syscall > 0);
}

uintptr_t sysc_call(syscall_t call, uintptr_t param0, uintptr_t param1) {
    if(!sysc_active()) {
        uintptr_t res;

        // TODO: sysenter, etc.?
        
        asm volatile(
            "\tint %1\n"
            "\tmov %%rax, %0\n"
            : "=a"(res) 
            : "i"(SYSC_INTERRUPT), "D"(call), "S"(param0), "d"(param1));

        return res;
    }

    switch(call) {
    case SysSchedule:
        sched_schedule();
        return 0;
    case SysYield:
        sched_yield();
        return 0;
    }

    return (uintptr_t)-1;
}

