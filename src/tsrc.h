/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"
#include "extp.h"

/** defines the maximum clock timeout milliseconds if no timer is present */
#define TSRC_MAX_TICK    10

/** Signature of a callback to be called on every timer iteration */
typedef void (*tsrc_cb_t)();

/** Describes a timesource, and how to handle it */
typedef struct {
    bool supported;                         /**< indicated whether timesource is supported. */
    bool (*init)(tsrc_cb_t callback);       /**< intialize the timesource for the given callback */
    bool (*schedule)(millis_t ms);          /**< schedule an interrupt when count reaches ms (absolute!) */
    millis_t (*current_ticks)();            /**< retrieve an arbitrary value of current ticks.
                                             * this is not related to "real time". */
} tsrc_t;

/** Signature of the timesource extension point function */
typedef tsrc_t* (*tsrc_extp_t)();

/**
 * Schedule a timer callback to be called once or periodically.
 *
 * @param callback  the function which will be called when the timeout expires.
 * @param ms        timeout in milliseconds.
 * @param oneshot   whether oneshot is requested, or periodic.
 */
bool tsrc_schedule(tsrc_cb_t callback, millis_t ms, bool oneshot);

