/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include <thread.h>
#include <process.h>
#include "cpu.h"

typedef struct {
    cpu_state_t state;

    spinlock_t lock;
    thread_t* thread;
} thr_context_t;

struct _tag_thread_t {
    tid_t id;
    thread_state_t state;
    process_t* parent;
    thr_context_t* context;
};

/**
 * Returns the threads current context.
 */
extern thr_context_t* x86_ctx_get();

/**
 * Sets the current context for this cpu.
 */
extern void x86_ctx_set(thr_context_t* ctx);
