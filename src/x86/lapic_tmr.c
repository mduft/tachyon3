/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "lapic.h"
#include "tsc.h"
#include "msr.h"
#include "cpu.h"
#include "rtc.h"

#include <tmr.h>
#include <extp.h>
#include <intr.h>
#include <log.h>

#define MAX_INIT_RETRIES    10000
#define CALIBRATE_COUNT     0xFFFFFFFF

// TODO: per CPU?
static tmr_cb_t _master;
static uint64_t _apic_ticks_per_us = 0;

static bool lapic_tmr_handler(interrupt_t* state) {
    trace("local apic timer: calling %p\n", _master);

    if(_master)
        _master();

    lapic_eoi();

    return true;
}

static bool lapic_tmr_calibrate(uint64_t systime, uint64_t increment) {
    static uint8_t ccount = 0;
    static uint64_t accu = 0;

    accu += increment;

    ccount++;
    info("lapic timer calibration after %d runs: %d t/usec\n", ccount, (accu / ccount));

    if(ccount == 2) {
        _apic_ticks_per_us = (accu / ccount);

        return false; // accurate enough. stop.
    }

    return true; // keep calibrating.
}

static bool lapic_tmr_init(tmr_cb_t master) {
    _master = master;

    APIC_REG(APIC_REG_INITIAL_COUNT) = 0;
    APIC_REG(APIC_REG_DIVIDE_CONFIG) = 0x1;

    bool enabled = intr_state();
    intr_enable();

    rtc_calibrate(lapic_tmr_calibrate);

    while(!_apic_ticks_per_us);

    if(!enabled)
        intr_disable();

    info("local apic calibrated to %ld ticks per micro-second\n", _apic_ticks_per_us);

    intr_add(IRQ_LAPIC_TIMER, lapic_tmr_handler);
    APIC_REG(APIC_REG_LVT_TIMER) = IRQ_LAPIC_TIMER;

    return true;
}

static bool lapic_tmr_sched(uint64_t ns) {
    APIC_REG(APIC_REG_INITIAL_COUNT) = 
        ((ns - (rtc_systime())) * ((_apic_ticks_per_us / 1000) + 1));
    return true;
}

static tmr_gen_t* lapic_tmr_extp() {
    static tmr_gen_t _lapic_tmr = {
        .init = lapic_tmr_init,
        .schedule = lapic_tmr_sched,
    };

    /* TODO: better solution ... ? */
    _lapic_tmr.supported = lapic_is_enabled();

    return &_lapic_tmr;
}

INSTALL_EXTENSION(EXTP_TIMERGEN, lapic_tmr_extp, "local apic timer");