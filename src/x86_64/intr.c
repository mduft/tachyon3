/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include <tachyon.h>

#include "reg.h"
#include "cpu.h"
#include "thread.h"
#include <log.h>

bool intr_state() {
    register uint64_t rflags = 0;

    asm volatile("pushf; popq %0" : "=r"(rflags));

    return (rflags & FL_IF);
}

void intr_disable() {
    asm volatile("cli");
    x86_64_ctx_get()->ifda_cnt++;
}

bool intr_enable(bool doIt) {
    register thr_context_t* ctx = x86_64_ctx_get();

    if(ctx->ifda_cnt > 0)
        ctx->ifda_cnt--;

    if(ctx->ifda_cnt == 0) {
        if(doIt)
            asm volatile("sti");

        return true;
    }

    return false;
}

