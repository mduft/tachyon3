/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "lapic.h"
#include "tsc.h"
#include "msr.h"
#include "cpu.h"
#include "rtc.h"

#include <tmr.h>
#include <extp.h>
#include <intr.h>
#include <log.h>
#include <systime.h>

#define MAX_INIT_RETRIES    10000
#define CALIBRATE_COUNT     0xFFFFFFFF
#define CALIBRATE_TURNS     8
#define TICKS_INCR_2_HZ(ticks, incr) (ticks * (1000000 / incr))

// TODO: per CPU?
static tmr_cb_t _master;
static volatile uint64_t _lapic_hz = 0;

static bool lapic_tmr_handler(interrupt_t* state) {
    if(_master)
        _master();

    lapic_eoi();
    return true;
}

static bool lapic_tmr_calibrate(uint64_t systime, uint64_t incr) {
    static size_t turns = 0;
    static uint64_t avg = 0;
    uint64_t ticks;

    if(turns != 0) {
        ticks = (CALIBRATE_COUNT - APIC_REG(APIC_REG_CURRENT_COUNT));

        if(!avg)
            avg = TICKS_INCR_2_HZ(ticks, incr);
        else
            avg = (avg + TICKS_INCR_2_HZ(ticks, incr)) / 2;

        if(turns == CALIBRATE_TURNS) {
            _lapic_hz = avg;
            return false;
        }
    }

    ++turns;
    APIC_REG(APIC_REG_INITIAL_COUNT) = CALIBRATE_COUNT;

    return true;
}

static bool lapic_tmr_init(tmr_cb_t master) {
    if(intr_state())
        fatal("interrupts may not be enabled in lapic_tmr_init()!\n");

    _master = master;

    // fastest possible rate: 0xB (1011) -> FSB tick rate. on modern
    // systems, this should allow near nanoseconds resolution.
    APIC_REG(APIC_REG_DIVIDE_CONFIG) = 0xB;
    uint8_t old_rate = rtc_set_rate(RTC_RATE_128HZ);
    rtc_calibrate(lapic_tmr_calibrate);

    // enable interrupts for this to work, disable them again afterwards.
    intr_enable(true);

    // wait for calibration to finish.
    while(!_lapic_hz)
        asm volatile("hlt");

    intr_disable();

    rtc_set_rate(old_rate);

    info("local apic timer calibrated to %ld hz\n", _lapic_hz);

    intr_add(IRQ_LAPIC_TIMER, lapic_tmr_handler);
    APIC_REG(APIC_REG_LVT_TIMER) = IRQ_LAPIC_TIMER;

    return true;
}

static bool lapic_tmr_sched(uint64_t us) {
    uint64_t cnt = (us * _lapic_hz) / 1000000;

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
