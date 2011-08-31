/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include <tachyon.h>

/**
 * Each systime source can provide a rough estimate of it's accuracy.
 * This is used to choose the systime source to use. Within one accuracy
 * level, it's first come, first serve.
 */
typedef enum {
    Poor,
    Good,
    Best
} systime_accuracy_t;

/**
 * Describes a systime source, and it's required functionality.
 */
typedef struct {
    void (*systime_init_func)();    /**< pointer to a (possibly) required init function (or NULL) */
    uint64_t (*systime_us_func)();  /**< pointer to the systime function */
    systime_accuracy_t accuracy;    /**< rough estimation of the systime source's accuracy */
} systime_desc_t;

/** 
 * Describes the extension point function, which is used to query
 * each systime source's attributes.
 */
typedef systime_desc_t const* (*systime_extp)();

/**
 * Initializes the system timer facilities.
 */
void systime_init();

/**
 * Retrieves the current system time. the system time is the time in
 * microseconds that the system timesource has been ticking (so since
 * system boot).
 *
 * @return the uptime in microseconds
 */
uint64_t systime();

/**
 * Stalls the calling thread for a specified amount of microseconds
 *
 * @param us    microseconds to (busy) wait.
 */
void systime_stall(uintptr_t us);
