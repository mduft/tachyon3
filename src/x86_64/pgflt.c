/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "intr.h"
#include <log.h>
#include <extp.h>
#include <stka.h>
#include <ksym.h>

#include <x86/idt.h>

#define ERRC_TRANS_AVAILABLE        0x01
#define ERRC_ACC_WRITE              0x02
#define ERRC_ACC_USER               0x04
#define ERRC_TRANS_RESVD_BIT        0x08
#define ERRC_INSTR_FETCH            0x10

static void pgflt_install();
bool pgflt_handler(interrupt_t* state);

INSTALL_EXTENSION(EXTP_PLATFORM_INIT, pgflt_install, "page fault handler");

static void pgflt_install() {
    intr_add(EX_PAGE_FAULT, pgflt_handler);
}

bool pgflt_handler(interrupt_t* state) {
    ksym_t const* sym = ksym_get((void*)state->ip);
    trace("page-fault at %p <%s> while %s %p\n",
        state->ip, sym ? sym->name : "unknown", ((state->code & ERRC_INSTR_FETCH) ? 
            "fetching instructions from" : ((state->code & ERRC_ACC_WRITE) ? 
                "writing to" : "reading from")), state->ctx->state.cr2);

    if(state->code & ERRC_TRANS_AVAILABLE) {
        if(state->code & ERRC_TRANS_RESVD_BIT) {
            fatal("a translation for the page was available, but a reserved\n"
                  "bit was set in one of the paging structures!\n");
        }
    } else {
        trace("no translation for the given page was available!\n");

        thr_context_t* context = state->ctx;

        if(!context->thread)
            fatal("no thread associated with current execution context!\n");

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

        // nothing more to do.
        thr_abort(context->thread);
        return true;
    }

    fatal("unexpectedly reached end of page fault handler\n");
}
