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
#define CALIBRATE_TURNS     64

// TODO: per CPU?
static tmr_cb_t _master;
static volatile uint64_t _apic_ticks_per_us = 0;

    static bool lapic_tmr_sched(uint64_t ns);
static bool lapic_tmr_handler(interrupt_t* state) {
    if(_master)
        _master();

    lapic_eoi();
    return true;
}

static bool lapic_tmr_calibrate(uint64_t systime, uint64_t increment) {
    static size_t ccount = 0;
    static uint64_t accu_ticks = 0;
    static uint64_t accu_ns = 0;
    uint64_t ticks;

    if(ccount != 0) {
        ticks = (CALIBRATE_COUNT - APIC_REG(APIC_REG_CURRENT_COUNT));
        accu_ticks += ticks;
        accu_ns += increment;
    }

    ccount++;
    if(ccount == CALIBRATE_TURNS) {
        _apic_ticks_per_us = accu_ticks / (accu_ns / 1000);

        info("lapic timer after %d runs (%d ns, %ld ticks): %ld ticks per us\n", 
            ccount, accu_ns, ticks, _apic_ticks_per_us);

        return false; // accurate enough. stop.
    }

    APIC_REG(APIC_REG_INITIAL_COUNT) = CALIBRATE_COUNT;
    return true; // keep calibrating.
}

static bool lapic_tmr_init(tmr_cb_t master) {
    _master = master;

    // fastest possible rate: 0xB (1011) -> FSB tick rate. on modern
    // systems, this should allow near nanoseconds resolution.
    APIC_REG(APIC_REG_DIVIDE_CONFIG) = 0xB;
    uint8_t old_rate = rtc_set_rate(RTC_RATE_256HZ);
    rtc_calibrate(lapic_tmr_calibrate);

    // wait for calibration to finish.
    while(!_apic_ticks_per_us);

    rtc_set_rate(old_rate);

    info("local apic calibrated to %ld ticks per micro-second\n", _apic_ticks_per_us);

    intr_add(IRQ_LAPIC_TIMER, lapic_tmr_handler);
    APIC_REG(APIC_REG_LVT_TIMER) = IRQ_LAPIC_TIMER | APIC_TM_PERIODIC;

    return true;
}

static bool lapic_tmr_sched(uint64_t ns) {
    uint64_t cnt = ((ns * (_apic_ticks_per_us)) / 1000);

    APIC_REG(APIC_REG_LVT_TIMER) = IRQ_LAPIC_TIMER;
    APIC_REG(APIC_REG_INITIAL_COUNT) = cnt;

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

uint32_t lapic_tmr_current() {
    return APIC_REG(APIC_REG_CURRENT_COUNT);
}

INSTALL_EXTENSION(EXTP_TIMERGEN, lapic_tmr_extp, "local apic timer");
