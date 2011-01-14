/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "tachyon.h"

/**
 * C lowest-level iterrupt handler. All interrupts are routed here
 * from the assembly stubs/trampolines.
 *
 * @param state state information for the interrupt.
 */
void intr_dispatch(interrupt_t* state) {

}

