/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "cpu.h"
#include "kheap.h"

cpu_context_t* cpu_context_init() {
    cpu_context_t* ctx = (cpu_context_t*)kheap_alloc(sizeof(cpu_context_t));

    spl_init(&ctx->lock);

    return ctx;
}

cpu_context_t* cpu_context_current() {
    // TODO: FIXME: cpu local storage!!!
    static cpu_context_t* context = NULL;

    if(!context)
        context = cpu_context_init();

    return context;
}

