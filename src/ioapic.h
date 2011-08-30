/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include <tachyon.h>
#include <intr.h>

/** default ioapic base address. TODO: dynamic detection, multiple ioapics! */
#define IOAPIC_BASE         0xFEC00000

#define IOAPIC_REG_ID       0x00
#define IOAPIC_ID(v)        (((v) >> 24) & 0xF)

#define IOAPIC_REG_VER      0x01
#define IOAPIC_VER(v)       ((v) & 0xFF)
#define IOAPIC_MAX_REDIR(v) (((v) >> 16) & 0xFF)

#define IOAPIC_REG_TABLE    0x10

#define IOAPIC_INT_MASKED       (1 << 16)
#define IOAPIC_INT_LEVEL        (1 << 15)
#define IOAPIC_INT_ACTIVELOW    (1 << 13)
#define IOAPIC_INT_LOGICAL      (1 << 11)

#define IOAPIC_DM_FIXED         (0)
#define IOAPIC_DM_LOW_PRIO      (1 << 8)
#define IOAPIC_DM_SMI           (2 << 8)
#define IOAPIC_DM_NMI           (4 << 8)
#define IOAPIC_DM_INIT          (5 << 8)
#define IOAPIC_DM_ExtINT        (7 << 8)

/**
 * Initializes the I/O APIC(s)
 */
void ioapic_init();

/**
 * Enables delivery of a specific interrupt to the given cpu.
 *
 * @param irq   the irq number to set.
 * @param cpuid enable delivery to this cpu.
 */
void ioapic_enable(uint8_t irq, uint32_t cpuid);

/**
 * Disables delivery of a specifiv interrupt to the given cpu.
 *
 * @param irq   the irq to disable.
 * @param cpuid the cpu to disable for.
 */
void ioapic_disable(uint8_t irq, uint32_t cpuid);

