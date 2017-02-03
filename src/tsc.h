/* Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include <tachyon.h>

/**
 * Reads the timestamp counter.
 *
 * @return the current timestamp counter value.
 */
uint64_t tsc_read();

/**
 * Reads the timestamp counter and the current CPUs ID
 * in a single atomic instruction.
 *
 * @param [out] timer   the timestamp counter value.
 * @param [out] cpuid   the current CPUs ID.
 */
void tsc_read_p(uint64_t* timer, uint32_t* cpuid);

/**
 * retrieves the calibrated speed that the TSC is running at
 * (estimated)
 */
uint64_t tsc_hz();

/**
 * Initializes the TSC. this is called from the RTC init code,
 * as the RTC depends on the TSC, and vice versa.
 */
void tsc_init();
