/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"
#include "thread.h"
#include "list.h"

/**
 * The descriptor of a process. Holds all relevant
 * information associated with it.
 */
typedef struct {
    spc_t space;        /**< the address space for a process */
    list_t* threads;    /**< list of associated threads */
} process_t;

/**
 * The core tachyon process descriptor.
 */
extern process_t tachyon;

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

