/* Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
 * This file is part of the 'tachyon' operating system. */

#include <syscall.h>
#include <extp.h>
#include <thread.h>
#include <log.h>
#include <sched.h>

uintptr_t sysc_call(interrupt_t* state, syscall_handler_t handler) {
    thread_t* thr = state->ctx->thread;

    if(!thr)
        fatal("no thread associated with execution context!\n");

    thr->syscall++;

    state->ctx->state.rax = handler(sysc_get_call(state),
        state->ctx->state.rsi, state->ctx->state.rdx);

    thr->syscall--;
    return state->ctx->state.rax;
}

void sysc_init() {
    intr_set_mode(SYSC_INTERRUPT, GateModeMultiHandler);
}

bool sysc_active() {
    thread_t* thr = thr_ctx_get()->thread;

    if(!thr)
        fatal("no thread associated with execution context!\n");

    return (thr->syscall > 0);
}

syscall_t sysc_get_call(interrupt_t* state) {
    return state->ctx->state.rdi;
}
