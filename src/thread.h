/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

/**
 * Describes the different states a thread can be in.
 */
typedef enum {
    Initial,    /**< initial state while creating the thread. */
    Runnable,   /**< state while thread is waiting to run */
    Running,    /**< state while thread is running */
    Waiting,    /**< thread is waiting for something */
    Done        /**< thread has exited, waiting for cleanup */
} thread_state_t;

/**
 * Describes a process, forward declared to avoid circular
 * includes process <-> thread
 */
struct _tag_process_t;

/**
 * Describes a threads cpu context. Forward declared as the
 * actual structure is platform dependant.
 */
struct _tag_thr_context_t;

/**
 * Describes a thread and its state (both CPU and thread state)
 */
typedef struct {
    tid_t id;                           /**< the threads id within it's process */
    thread_state_t state;               /**< the threads execution state */
    struct _tag_process_t* parent;      /**< the threads parent process */
    struct _tag_thr_context_t* context; /**< the threads associated cpu context */
} thread_t;


/**
 * Defines the type for a threads entry point.
 */
typedef void (*thread_start_t)();

/**
 * Creates a new thread within the given process.
 *
 * @param process   the parent process for the thread.
 * @return          the new thread
 */
thread_t* thr_create(struct _tag_process_t* process, thread_start_t entry);

/**
 * Frees all associated resources for the given thread,
 * and removes it from the parent's process.
 *
 * @param thread    the thread to destroy
 * @return          always NULL
 */
thread_t* thr_delete(thread_t* thread);

/**
 * Switches the current cpu to another thread.
 *
 * @param thread    the thread to switch to.
 * @return          the previously running thread.
 */
thread_t* thr_switch(thread_t* target);
