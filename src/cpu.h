/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"
#include "thread.h"
#include "spl.h"

/**
 * Describes the current context of a CPU.
 */
typedef struct {
    thread_t* thread;   /**< the currently executing thread */
    spinlock_t lock;    /**< the context' lock */
} cpu_context_t;

/**
 * Describes the platform dependant cpu state.
 */
typedef struct _tag_cpu_state_t cpu_state_t;

/**
 * Initializes a cpu context. This is done once per cpu
 * only while initializing cpus.
 *
 * @return  the new context.
 */
cpu_context_t* cpu_context_new();

/**
 * Retrieve the current CPUs context.
 *
 * @return  the cpus context.
 */
cpu_context_t* cpu_context_current();

/**
 * Saves the current state of the current cpu into a
 * given storage location.
 *
 * @param storage   the storage for the state.
 */
void cpu_state_save(cpu_state_t* storage);

/**
 * Restores the state of the current cpu from the
 * given storage.
 *
 * @param storage   the storage to restore from.
 */
void cpu_state_restore(cpu_state_t* storage);
