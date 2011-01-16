/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "tachyon.h"
#include "log.h"

/**
 * C lowest-level iterrupt handler. All interrupts are routed here
 * from the assembly stubs/trampolines.
 *
 * @param state state information for the interrupt.
 */
void intr_dispatch(interrupt_t* state) {
    fatal("unhandled interrupt %d in %p\n", state->num, state->ip);
}

