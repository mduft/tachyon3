/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"
#include "thread.h"

#define IRQ_BASE    0x24
#define IRQ_NUM(x)  (IRQ_BASE + x)

typedef enum {
    GateModeSingleHandler   = 0x1,
    GateModeMultiHandler    = 0x2,
    GateModeNotifyAll       = 0x4
} gatemode_t;

typedef struct {
    thr_context_t* ctx; /**< interrupted threads thread context */

    uintptr_t   num;    /**< the interrupt number */
    uintptr_t   code;   /**< the error code, or zero if none */
    uintptr_t   ip;     /**< the interrupted location */
    uintptr_t   cs;     /**< the code segment of the interrupted location. */
    uintptr_t   flags;  /**< the eflags/rflags of the interrupted thread. */

    uintptr_t   sp;     /**< the original stack pointer */
    uintptr_t   ss;     /**< the original stack segment */
} interrupt_t;


typedef bool (*intr_handler_t)(interrupt_t* info);

/**
 * Enable interrupts. This decrements a counter until
 * it reaches zero, and then really enables them.
 *
 * @param doIt  whether to really enable interrupts
 * @return      whether interrupts ought to be enabled
 */
bool intr_enable(bool doIt);

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

/**
 * Register an interrupt handler for a specific interrupt
 * gate.
 *
 * @param gate      the gate to register a handler for.
 * @param handler   the handler callback.
 */
void intr_add(uint16_t gate, intr_handler_t handler);

/**
 * Removes an interrupt handler for a specific interrupt
 * gate.
 *
 * @param gate      the gate to remove the handler from.
 * @param handler   the handler callback.
 */
void intr_remove(uint16_t gate, intr_handler_t handler);

/**
 * Sets the mode for a gate. Any already registered handler
 * will be lost on mode switch!
 *
 * @param gate      the gate to modify.
 * @param mode      the new gate mode to set. default is GateModeSingleHandler
 */
void intr_set_mode(uint16_t gate, gatemode_t mode);

/**
 * Dumps the IDT to trace for debugging purposes.
 */
void intr_dump_idt();
