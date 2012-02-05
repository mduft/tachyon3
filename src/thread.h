/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"
#include "stka.h"
#include "cpu.h"

/**
 * Describes the different states a thread can be in.
 */
typedef enum {
    Initial,    /**< initial state while creating the thread. */
    Runnable,   /**< state while thread is waiting to run */
    Running,    /**< state while thread is running */
    Yielded,    /**< thread has been yielded, and should not continue to run this round */
    Waiting,    /**< thread is waiting for something */
    Aborting,   /**< thread still running, but aborting at the next chance */
    Aborted,    /**< thread has been forcefully aborted */
    Exited      /**< thread has exited, waiting for cleanup */
} thread_state_t;

/**
 * Priority of a thread or a process.
 */
typedef enum {
    Idle,
    Lowest,
    Low,
    Normal,
    High,
    Highest,
    Kernel,
    MaxPrio
} priority_t;

/**
 * Describes a process, forward declared to avoid circular
 * includes process <-> thread
 */
struct _tag_process_t;

/** Defined below, forward for context */
struct _tag_thread_t;

/**
 * Describes a threads cpu context. Forward declared as the
 * actual structure is platform dependant.
 */
typedef struct _tag_thr_context_t {
    cpu_state_t state;

    uint8_t ifda_cnt;

    spinlock_t lock;
    struct _tag_thread_t* thread;
} thr_context_t;

/**
 * Describes a thread and its state (both CPU and thread state)
 */
typedef struct _tag_thread_t {
    tid_t id;                           /**< the threads id within it's process */
    thread_state_t state;               /**< the threads execution state */
    priority_t priority;                /**< the threads priority, inherited from 
                                         *   the parent process, may be overridden. */
    stack_t* stack;                     /**< the threads stack */
    struct _tag_process_t* parent;      /**< the threads parent process */
    thr_context_t* context;             /**< the threads associated cpu context */
    uint8_t syscall;                    /**< indicates whether the thread is in a syscall */
    uint64_t preempt_at;                /**< absolute point in system time to interrupt thread at latest */
} thread_t;

/* forward for thread start */
struct _tag_uapi_desc_t;

/**
 * Defines the type for a threads entry point.
 */
typedef void (*thread_start_t)(struct _tag_uapi_desc_t const*);

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

/**
 * Retrieves the currently running thread for the current CPU.
 *
 * @return          the currently running thread.
 */
thread_t* thr_current();

/**
 * Marks a thread for abortion at the next
 * scheduling round.
 *
 * @param thread    the thread to abort forcefully.
 */
void thr_abort(thread_t* thread);

/**
 * Returns the threads current context.
 * @attention implemented in context.S
 */
extern thr_context_t* thr_ctx_get();

/**
 * Sets the current context for this cpu.
 * @attention implemented in context.S
 */
extern void thr_ctx_set(thr_context_t* ctx);
