/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"
#include "thread.h"

/**
 * Describes the current context of a CPU.
 */
typedef struct {
    thread_t* thread;   /**< the currently executing thread */
} cpu_context_t;

/**
 * Retrieve the current CPUs context.
 *
 * @return  the cpus context.
 */
cpu_context_t* cpu_get_context();

