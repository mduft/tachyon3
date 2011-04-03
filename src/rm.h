/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

/**
 * Describes one general purpose register, which is
 * accessbile either as dword, word or two byes.
 */
typedef union {
    uint32_t dword;
    uint16_t word;
    struct {
        uint8_t lobyte;
        uint8_t hibyte;
    };
} rm_reg_t;

/**
 * Describes the current real mode call state.
 */
typedef struct {
    rm_reg_t    ax, bx, cx, dx, sp, bp, si, di;
    uint16_t    ss, ds, es, cs, ip, fl;
} rm_state_t;

/**
 * Initializes the real mode subsystem. This allocates
 * the necessary virtual memory mappings for the low
 * memory.
 */
void rm_init();

/**
 * Calls a real mode interrupt.
 *
 * @param vec   the interrupt number to call.
 * @param state the cpu state to emulate.
 * @return      true on success, false otherwise.
 */
bool rm_int(uint8_t vec, rm_state_t* state);

/**
 * Executes any real mode code, given by the ip field
 * in the given cpu state.
 *
 * @param state the cpu state to emulate.
 * @return      true on success, false otherwise.
 */
bool rm_call(rm_state_t* state);
