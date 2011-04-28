/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "cpu.h"
#include <intr.h>

/**
 * describes the available information when an interrupt happens.
 */
struct _tag_interrupt_t {
    struct _tag_thr_context_t* ctx; /**< interrupted threads thread context */

    uintptr_t   num;    /**< the interrupt number */
    uintptr_t   code;   /**< the error code, or zero if none */
    uintptr_t   ip;     /**< the interrupted location */
    uintptr_t   cs;     /**< the code segment of the interrupted location. */
    uintptr_t   flags;  /**< the eflags/rflags of the interrupted thread. */

    uintptr_t   sp;     /**< (only on ring change). the original stack pointer */
    uintptr_t   ss;     /**< (only on ring change). the original stack segment */
};

