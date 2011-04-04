/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include <cpu.h>
#include <kheap.h>

cpu_context_t* cpu_get_context() {
    // TODO: FIXME: cpu local storage!!!
    static cpu_context_t* context = NULL;

    if(context == NULL) {
        context = (cpu_context_t*)kheap_alloc(sizeof(cpu_context_t));
    }

    return context;
}

