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
 * The descriptor of a thread. Holds all relevant
 * information associated with it.
 */
typedef struct {
    thread_state_t;
    void* stack;
} thread_t;

