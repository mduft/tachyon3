/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

#define SYSC_INTERRUPT  50

typedef enum {
    SysSchedule,    /**< re-schedule, no parameters */
    SysYield,       /**< yield current thread */
} syscall_t;

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
 * run a given system call. either through an interrupt,
 * a sysenter call or a direct call, depending on whether
 * another system call is currently running.
 *
 * @param call      the call to execute.
 * @param param0    first value to pass, depends on call.
 * @param param1    second value to pass, depends on call.
 * @return          depends on call.
 */
uintptr_t sysc_call(syscall_t call, uintptr_t param0, uintptr_t param1);
