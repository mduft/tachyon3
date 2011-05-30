/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"
#include "extp.h"

/** Signature of a callback to be called on every timer iteration */
typedef void (*tsrc_cb_t)();

/**
 * Schedule a timer callback to be called once or periodically.
 *
 * @param callback  the function which will be called when the timeout expires.
 * @param ms        timeout in milliseconds.
 * @param oneshot   whether oneshot is requested, or periodic.
 */
bool tsrc_schedule(tsrc_cb_t callback, ssize_t ms, bool oneshot);

