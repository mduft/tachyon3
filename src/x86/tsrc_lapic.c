/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "lapic.h"
#include "tsc.h"
#include "msr.h"
#include "cpu.h"

#include <tsrc.h>
#include <extp.h>
#include <intr.h>
#include <log.h>

#define MAX_INIT_RETRIES    10000
#define CALIBRATE_COUNT     0xFFFFFFFF

// TODO: per CPU?
static tsrc_cb_t _master;
static uintmax_t _ticks_per_ms;
static uintmax_t _tsc_ticks_per_ms;

static bool tsrc_lapic_handler(interrupt_t* state) {
    _master();
    return true;
}

static void tsrc_lapic_calibrate() {
    // TODO!
    APIC_REG(APIC_REG_DIVIDE_CONFIG) = 0x1;
    _ticks_per_ms = 100000;
    _tsc_ticks_per_ms = 2000000;
}

static bool tsrc_lapic_init(tsrc_cb_t master) {
    _master = master;

    APIC_REG(APIC_REG_INITIAL_COUNT) = 0;

    trace("timestamp counter for lapic timer goes at %d ticks per ms\n", _ticks_per_ms);

    tsrc_lapic_calibrate();

    intr_add(IRQ_LAPIC_TIMER, tsrc_lapic_handler);

    APIC_REG(APIC_REG_LVT_TIMER) = IRQ_LAPIC_TIMER;

    return true;
}

static millis_t tsrc_lapic_millis() {
    // TODO: some better millisecond source...
    return tsc_read() / _tsc_ticks_per_ms;
}

static bool tsrc_lapic_sched(millis_t deadline) {
    APIC_REG(APIC_REG_INITIAL_COUNT) = ((deadline - (tsrc_lapic_millis())) * _ticks_per_ms);
    return true;
}

static tsrc_t* tsrc_lapic_extp() {
    static tsrc_t _lapic_tsrc = {
        .init = tsrc_lapic_init,
        .schedule = tsrc_lapic_sched,
        .current_millis = tsrc_lapic_millis,
    };

    /* TODO: better solution ... ? */
    _lapic_tsrc.supported = lapic_is_enabled();

    return &_lapic_tsrc;
}

INSTALL_EXTENSION(EXTP_TIMESOURCE, tsrc_lapic_extp, "local apic timer");
