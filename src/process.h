/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"
#include "thread.h"
#include "list.h"
#include "spl.h"
#include "heap.h"

/**
 * The descriptor of a process. Holds all relevant
 * information associated with it.
 */
typedef struct _tag_process_t {
    pid_t id;           /**< the process' unique id within the system */
    spc_t space;        /**< the root of the process' address space */
    uint8_t ring;       /**< the ring the process lives in (kernel-space, user-space, ...) */
    uint8_t priority;   /**< the process' priority */
    spinlock_t lock;    /**< a process lock, locked whenever the process is modified */
    tid_t ctid;         /**< the current thread id counter - each new thread gets ctid++ */
    list_t* threads;    /**< the list of all currently alive threads */
    heap_t heap;        /**< the process' heap for it's address space */
    heap_t stack_heap;  /**< heap to allocate stack space from */
} process_t;

/**
 * Creates a new process descriptor and allocates a new
 * address space for it.
 *
 * @return the new process' descriptor.
 */
process_t* prc_new();

/**
 * Frees any resources associated with a process.
 *
 * @param prc the process to delete
 * @return always NULL
 */
process_t* prc_delete(process_t* prc);

/**
 * Retrieves the next valid thread-id for the given
 * process.
 *
 * @param prc   the process to get a free id for.
 * @return      the thread id.
 */
tid_t prc_next_tid(process_t* prc);

/**
 * Adds a thread to the process' thread list. This
 * should be called only by thread implementations.
 *
 * @param prc   the target process.
 * @param thr   the thread to add.
 */
void prc_add_thread(process_t* prc, thread_t* thr);

/**
 * Removes a thread from the given process' thread list.
 *
 * @param prc   the target process.
 * @param thr   the thread to remove.
 */
void prc_remove_thread(process_t* prc, thread_t* thr);
