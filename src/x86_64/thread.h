/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include <thread.h>
#include <process.h>
#include "cpu.h"

typedef struct _tag_thr_context_t {
    x86_64_cpu_state_t state;

    uint8_t ifda_cnt;

    spinlock_t lock;
    thread_t* thread;
} thr_context_t;

/**
 * Returns the threads current context.
 * @attention implemented in context.S
 */
extern thr_context_t* x86_64_ctx_get();

/**
 * Sets the current context for this cpu.
 * @attention implemented in context.S
 */
extern void x86_64_ctx_set(thr_context_t* ctx);
