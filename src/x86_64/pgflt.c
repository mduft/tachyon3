/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "intr.h"
#include <log.h>
#include <extp.h>

#include <x86/idt.h>

#define ERRC_TRANS_AVAILABLE        0x01
#define ERRC_ACC_WRITE              0x02
#define ERRC_ACC_USER               0x04
#define ERRC_TRANS_RESVD_BIT        0x08
#define ERRC_INSTR_FETCH            0x10

static void pgflt_install();
static bool pgflt_handler(interrupt_t* state);

INSTALL_EXTENSION(EXTP_KINIT, pgflt_install, "page fault handler");

static void pgflt_install() {
    intr_add(EX_PAGE_FAULT, pgflt_handler);
}

static bool pgflt_handler(interrupt_t* state) {
    error("page-fault at %p while %s %p\n",
        state->ip, ((state->code & ERRC_INSTR_FETCH) ? 
            "fetching instructions from" :
            ((state->code & ERRC_ACC_WRITE) ? 
                "writing to" : "reading from")), 
        state->ctx->state.cr2);

    if(state->code & ERRC_TRANS_AVAILABLE) {
        if(state->code & ERRC_TRANS_RESVD_BIT) {
            error("a translation for the page was available, but a reserved\n"
                  "bit was set in one of the paging structures!\n");
        }
    } else {
        error("no translation for the given page was available!\n");
    }

    /* at the moment, we're not "handling" this, but only
     * give some useful information to the developer... */
    return false;
}
