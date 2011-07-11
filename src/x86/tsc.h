/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
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
