/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "intr.h"
#include <log.h>
#include <extp.h>
#include <stka.h>
#include <ksym.h>

#include "idt.h"
#include "thread.h"
#include "process.h"

#define ERRC_TRANS_AVAILABLE        0x01
#define ERRC_ACC_WRITE              0x02
#define ERRC_ACC_USER               0x04
#define ERRC_TRANS_RESVD_BIT        0x08
#define ERRC_INSTR_FETCH            0x10

bool pgflt_handler(interrupt_t* state);

void pgflt_init() {
    intr_add(EX_PAGE_FAULT, pgflt_handler);
}

bool pgflt_handler(interrupt_t* state) {
    ksym_t const* sym = ksym_get((void*)state->ip);
    thr_context_t* context = state->ctx;
    trace("page-fault at %p <%p:%s + 0x%x> while %s %p\n",
        state->ip,sym ? sym->addr : 0x0, sym ? sym->name : "unknown",
        sym ? (state->ip - sym->addr) : 0, ((state->code & ERRC_INSTR_FETCH) ? 
            "fetching instructions from" : ((state->code & ERRC_ACC_WRITE) ? 
                "writing to" : "reading from")), state->ctx->state.cr2);

    trace("CPU registers after fault:\n"
        " RAX: %p, RBX: %p\n"
        " RCX: %p, RDX: %p\n"
        " RBP: %p, RSP: %p\n", state->ctx->state.rax,
        state->ctx->state.rbx, state->ctx->state.rcx,
        state->ctx->state.rdx, state->ctx->state.rbp,
        state->ctx->state.rsp);

    if(!context->thread) {
        fatal("no thread associated with current execution context!\n");
    } else {
        trace("fault occured in thread %d, process %d\n", context->thread->id, context->thread->parent->id);
    }

    if(state->code & ERRC_TRANS_AVAILABLE) {
        if(state->code & ERRC_TRANS_RESVD_BIT) {
            error("a translation for the page was available, but a reserved\n"
                  "bit was set in one of the paging structures!\n");
        }
        if(state->code & ERRC_ACC_USER) {
            error("present page (0x%lx) not accessible in user mode\n", state->ctx->state.cr2);
        }
        fatal("unknwon error when accessing present page\n");
    } else {
        trace("no translation for the given page was available!\n");
        stack_t* stk = context->thread->stack;

        if(context->state.cr2 >= stk->guard && context->state.cr2 <= stk->top) {
            trace("looks like a stack grow request, trying to enlarge stack\n");

            if(!context->thread->parent)
                fatal("no process associated with current thread!\n");

            stack_allocator_t* stka = context->thread->parent->stka;

            if(stka_pgflt(stka, stk, context->state.cr2)) {
                trace("page fault handled by growing the stack for thread %d in process %d\n",
                    context->thread->id, context->thread->parent->id);

                return true;
            } else {
                warn("growing stack for thread %d in process %d failed; stack is %d bytes large!\n",
                    context->thread->id, context->thread->parent->id, stk->top - stk->mapped);
            }
        }

        error("don't know anything more to do. page-fault @ %p (code 0x%lx) without resolution!\n", state->ip, state->code);
    }
    
    // nothing more to do.
    thr_abort(context->thread);
    return true;
}
