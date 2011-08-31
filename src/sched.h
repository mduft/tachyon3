/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"
#include "thread.h"

#define SCHED_TIMESLICE_US (1*1000)

/**
 * Does some basic initialization for the scheduler.
 */
void sched_init();

/**
 * Does the real scheduling work. For all registered threads,
 * find one that is runnable, and make it current.
 *
 * Warning: May re-schedule the same thread if it's timeslice
 * is not used up, and the thread is still in a runable state!
 */
void sched_schedule();

/**
 * Yields the current thread, giving up it's remaining timeslice.
 * Calls sched_schedule() to make another thread current.
 */
void sched_yield();

/**
 * Starts the scheduler by setting up timers, etc. The calling
 * thread is halted. Make sure that the calling thread is either
 * in a non-runable state, or deals with this returning.
 */
void sched_start();

/**
 * Register a thread with the scheduler. The thread must be in
 * a runable state!
 *
 * @param thread    the thread to register.
 */
void sched_add(thread_t* thread);

/**
 * Removes a thread from the scheduler. The thread won't be
 * scheduled anymore, regardless of it's state. This won't
 * destroy the thread, but only removes it from the queue.
 *
 * @param thread    the thread to remove.
 */
void sched_remove(thread_t* thread);

/**
 * Makes a thread wait on a specified object id. object id
 * may be an arbitrary value. A call to sched_wake with the
 * same id will return all threads waiting on this id to a
 * runable state.
 *
 * @param thread    the thread to block.
 * @param oid       the object-id to block on.
 */
void sched_wait(thread_t* thread, uintptr_t oid);

/**
 * Wakes all threads that are blocking on a specific object-id.
 *
 * @param oid       the object-id to wake threads for.
 */
void sched_wake(uintptr_t oid);

