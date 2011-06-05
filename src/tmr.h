/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"
#include "extp.h"

/** defines the maximum timeout nanoseconds if no timer is present */
#define TMR_MAX_TIMEOUT    (10 * 1000)

/** Signature of a callback to be called on every timer iteration */
typedef void (*tmr_cb_t)();

/** Describes a timesource, and how to handle it */
typedef struct {
    bool supported;                         /**< indicated whether timesource is supported. */
    bool (*init)(tmr_cb_t callback);        /**< intialize the timesource for the given callback */
    bool (*schedule)(uint64_t ns);          /**< schedule an interrupt when count reaches ms (absolute!) */
} tmr_gen_t;

/** Signature of the timesource extension point function */
typedef tmr_gen_t* (*tmr_extp_t)();

/**
 * Schedule a timer callback to be called once or periodically.
 *
 * @param callback  the function which will be called when the timeout expires.
 * @param ns        timeout in nanoseconds.
 * @param oneshot   whether oneshot is requested, or periodic.
 */
bool tmr_schedule(tmr_cb_t callback, uint64_t ns, bool oneshot);

