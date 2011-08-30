/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#define IA32_APIC_BASE              0x0000001B
#define IA32_TSC_DEADLINE           0x000006E0
#define IA32_EFER                   0xC0000080

#ifndef __ASM__

#include <tachyon.h>

/**
 * Reads a model-specific register from the current CPU.
 *
 * @param msr   the msr's ID (see above).
 * @return      the value of the register
 */
uint64_t msr_read(uint32_t msr);

/**
 * Writes a model-specific register to the current CPU.
 *
 * @param msr   the register ID to write to.
 * @param value the value to write to the register.
 */
void msr_write(uint32_t msr, uint64_t value);

#endif

