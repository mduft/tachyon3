/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"
#include "intr.h"

#define SYSC_INTERRUPT  50

typedef enum {
    SysSchedule,    /**< re-schedule, no parameters */
    SysYield,       /**< yield current thread */
    SysThrExit,     /**< abort thread */
    SysLog,         /**< access to the kernel logging */
} syscall_t;

/** handler callback for syscall */
typedef uintptr_t (*syscall_handler_t)(syscall_t call, uintptr_t param0, uintptr_t param1);

/**
 * Initializes the system call handlers (interrupt, sysenter, ...)
 */
void sysc_init();

/**
 * Returns whether a syscall is currently active.
 *
 * @return true if a syscall is running, false otherwise.
 */
bool sysc_active();

/**
 * calls the given syscall handler, and handles the correct
 * reading/setting CPU registers for the calling thread.
 *
 * it is valid to pass a handler which takes less arguments
 * than specified by syscall_handler_t or which returns void.
 * 
 * this way it is possible to feed void(void) methods directly
 * to sysc_call without the need for a wrapper function.
 *
 * @param state the interrupt CPU state.
 * @param handler the handler to call.
 * @return the return code of the system call. note that the result is
 *         also written to the RAX register of the calling thread.
 */
uintptr_t sysc_call(interrupt_t* state, syscall_handler_t handler);

/**
 * Returns the system call that is requested by the given
 * state captured from a system call interrupt.
 *
 * @return the system call id for the current call.
 */
syscall_t sysc_get_call(interrupt_t* state);
