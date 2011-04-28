/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include <spl.h>
#include <thread.h>

typedef struct {
    uintptr_t rip;
} cpu_state_t;

typedef struct {
    cpu_state_t state;

    spinlock_t lock;
    thread_t* thread;
} cpu_context_t;


/**
 * Returns the cpus current context.
 */
extern cpu_context_t* x86_64_ctx_get();

