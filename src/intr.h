/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

/**
 * Enable interrupts. This decrements a counter until
 * it reaches zero, and then really enables them.
 */
void intr_enable();

/**
 * Disables interrupts. This increments a counter, and
 * only disables if the counter was zero.
 */
void intr_disable();

/**
 * Retrieves the interrupt state on the current cpu.
 *
 * @return true if delivery is on, false otherwise.
 */
bool intr_state();

