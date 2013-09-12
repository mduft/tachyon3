/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include <syscall.h>
#include <extp.h>
#include <thread.h>
#include <log.h>
#include <sched.h>

#include "intr.h"
#include "uapi.h"

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

void sysc_init() {
    intr_add(SYSC_INTERRUPT, sysc_handler);
}

bool sysc_active() {
    thread_t* thr = thr_ctx_get()->thread;

    if(!thr)
        fatal("no thread associated with execution context!\n");

    return (thr->syscall > 0);
}

uintptr_t sysc_call(syscall_t call, uintptr_t param0, uintptr_t param1) {
    if(!sysc_active()) {
        return uapi_sysc_call(call, param0, param1);
    }

    /* TODO: find a more dynamic way to wire this? */
    switch(call) {
    case SysSchedule:
        sched_schedule();
        return 0;
    case SysYield:
        sched_yield();
        return 0;
    case SysThrExit:
        thr_exit(thr_current());
        return 0;
    case SysLog:
        {
            log_level_t level = (log_level_t)param0;
            char const* str = (char const*)param1;
            log_write(level, str);
            return 0;
        }
    }

    return (uintptr_t)-1;
}

