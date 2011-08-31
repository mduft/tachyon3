/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "tsc.h"
#include "rtc.h"

#include <extp.h>
#include <log.h>
#include <intr.h>

#define CALIBRATE_TURNS 8
#define TICKS_INCR_2_HZ(ticks, incr) (ticks * (1000000000 / incr))

static volatile uint64_t _tsc_hz = 0;

uint64_t tsc_read() {
    register uint64_t c = 0;

    intr_disable();

    asm volatile("rdtsc; shl $31, %%rbx; or %%rbx, %0;"
        : "=a"(c) :: "%rbx");

    intr_enable(true);

    return c;
}

void tsc_read_p(uint64_t* timer, uint32_t* cpuid) {
    intr_disable();

    asm volatile("rdtscp; shl $31, %%rbx; or %%rbx, %0;"
        : "=a"(*timer), "=c"(*cpuid));

    intr_enable(true);
}

uint64_t tsc_hz() {
    return _tsc_hz;
}

static bool tsc_calibrate(uint64_t time, uint64_t incr) {
    static uint64_t ltsc = 0;
    static uint64_t avg = 0;
    static size_t turns = 0;
    uint64_t tsc = tsc_read();

    if(ltsc != 0) {
        uint64_t ticks = tsc - ltsc;

        if(!avg)
            avg = TICKS_INCR_2_HZ(ticks, incr);
        else
            avg = (avg + TICKS_INCR_2_HZ(ticks, incr)) / 2;

        if(turns == CALIBRATE_TURNS) {
            _tsc_hz = avg;
            return false;
        }
    }

    ++turns;
    ltsc = tsc;

    return true;
}

void tsc_init() {
    if(intr_state())
        fatal("interrupts may not be enabled in tsc_init()!\n");

    uint8_t old_rate = rtc_set_rate(RTC_RATE_128HZ);
    rtc_calibrate(tsc_calibrate);

    intr_enable(true);

    while(!_tsc_hz)
        asm volatile("hlt");

    intr_disable();

    rtc_set_rate(old_rate);

    info("tsc calibrated to %ld hz\n", _tsc_hz);
}


