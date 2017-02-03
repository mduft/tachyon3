/* Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include <tachyon.h>
#include <intr.h>

// -2 for the local APIC interrupts, which are not "real" IRQs
#define IRQ_SPURIOUS            IRQ_NUM(-3)
#define IRQ_ERROR               IRQ_NUM(-2)
#define IRQ_LAPIC_TIMER         IRQ_NUM(-1)

#define APIC_GLOBAL_ENABLE      (1 << 11)
#define APIC_BSP                (1 << 8)

#define APIC_REG_ID             0x20
#define APIC_REG_VERSION        0x30
#define APIC_REG_TPR            0x80
#define APIC_REG_APR            0x90
#define APIC_REG_CPR            0xa0
#define APIC_REG_EOI            0xb0
#define APIC_REG_REMOTE_READ    0xc0
#define APIC_REG_LOGIC_DEST     0xd0
#define APIC_REG_DEST_FORMAT    0xe0
#define APIC_REG_SV             0xf0

#define APIC_REG_ISR0           0x100
#define APIC_REG_ISR1           0x110
#define APIC_REG_ISR2           0x120
#define APIC_REG_ISR3           0x130
#define APIC_REG_ISR4           0x140
#define APIC_REG_ISR5           0x150
#define APIC_REG_ISR6           0x160
#define APIC_REG_ISR7           0x170

#define APIC_REG_TMR0           0x180
#define APIC_REG_TMR1           0x190
#define APIC_REG_TMR2           0x1a0
#define APIC_REG_TMR3           0x1b0
#define APIC_REG_TMR4           0x1c0
#define APIC_REG_TMR5           0x1d0
#define APIC_REG_TMR6           0x1e0
#define APIC_REG_TMR7           0x1f0

#define APIC_REG_IRR0           0x200
#define APIC_REG_IRR1           0x210
#define APIC_REG_IRR2           0x220
#define APIC_REG_IRR3           0x230
#define APIC_REG_IRR4           0x240
#define APIC_REG_IRR5           0x250
#define APIC_REG_IRR6           0x260
#define APIC_REG_IRR7           0x270

#define APIC_REG_ERROR_STATUS   0x280
#define APIC_REG_LVT_CMCI       0x2f0

#define APIC_REG_ICR0           0x300
#define APIC_REG_ICR1           0x310

#define APIC_REG_LVT_TIMER      0x320
#define APIC_REG_LVT_THERMAL    0x330
#define APIC_REG_LVT_PERF       0x340
#define APIC_REG_LVT_LINT0      0x350
#define APIC_REG_LVT_LINT1      0x360
#define APIC_REG_LVT_ERROR      0x370

#define APIC_REG_INITIAL_COUNT  0x380
#define APIC_REG_CURRENT_COUNT  0x390
#define APIC_REG_DIVIDE_CONFIG  0x3e0

#define APIC_SV_ENABLE          (1 << 8)
#define APIC_SV_NO_EOI_BCAST    (1 << 12)

#define APIC_TM_PERIODIC        (1 << 17)
#define APIC_TM_TSC_DEADL       (2 << 17)

#define APIC_TIMER_MASKED       (1 << 16)
#define APIC_TIMER_VECTOR       0x20

#define APIC_LVT_SEND_PENDING   (1 << 12)

#define APIC_DM_FIXED           0x0
#define APIC_DM_SMI             0x2
#define APIC_DM_NMI             0x4
#define APIC_DM_INIT            0x5
#define APIC_DM_EXT             0x7

#define APIC_LVT_TRIGGER_LEVEL  (1 << 15)
#define APIC_LVT_REMOTE_IRR     (1 << 14)
#define APIC_LVT_IN_PIN_POL     (1 << 13)
#define APIC_LVT_MASKED         (1 << 17)

#define APIC_REG(x) *((uint32_t*)(APIC_VIRTUAL + x))

/**
 * Initializes the CPUs local APIC. Also initializes the TSC
 * registers, so that RDTSCP returns the correct APIC ID.
 */
void lapic_init();

/**
 * Retrieves the CPUs APIC id. This may be done by calling RDTSCP,
 * thus lapic_init() has to be called before this.
 *
 * @returns the unique CPU identifier.
 */
cpuid_t lapic_cpuid();

/**
 * Acknowledges a currently delivering interrupt with the controller.
 */
void lapic_eoi();

/**
 * Check whether the current CPU is the bootstrap processor.
 *
 * @return true if BSP, false if AP (Application Processor).
 */
bool lapic_is_bsp();

/**
 * Checks whether the local APIC is enabled.
 *
 * @return true if enabled, false otherwise.
 */
bool lapic_is_enabled();
