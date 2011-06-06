/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include <x86/tsc.h>
#include <x86/rtc.h>

#include <extp.h>
#include <log.h>

#define CALIBRATE_TURNS 64

static volatile uint64_t _tsc_ticks_per_ns = 0;

uint64_t tsc_read() {
    register uint64_t c asm("rax") = 0;

    asm volatile("rdtsc; shl $31, %%rbx; or %%rbx, %0;"
        : "=a"(c) :: "%rbx");

    return c;
}

void tsc_read_p(uint64_t* timer, uint32_t* cpuid) {
    asm volatile("rdtscp; shl $31, %%rbx; or %%rbx, %0;"
        : "=a"(*timer), "=c"(*cpuid));
}

uint64_t tsc_rate() {
    return _tsc_ticks_per_ns;
}

static bool tsc_calibrate(uint64_t systime, uint64_t increment) {
    static uint64_t last_ticks = 0;
    static uint64_t accu_ticks = 0;
    static uint64_t accu_ns = 0;
    static size_t ccount = 0;
    uint64_t ticks = tsc_read();

    if(last_ticks) {
        accu_ticks += ticks - last_ticks;
        accu_ns += increment;
    }

    ccount++;
    if(ccount == CALIBRATE_TURNS) {
        _tsc_ticks_per_ns = accu_ticks / (accu_ns);

        if(_tsc_ticks_per_ns == 0)
            _tsc_ticks_per_ns++; // make it at least one tick per ns.

        info("tsc rate after %d runs (%d ns, %ld ticks): %ld ticks per us\n",
            ccount, accu_ns, accu_ticks, _tsc_ticks_per_ns);

        return false;
    }

    last_ticks = ticks;

    return true;
}

static void tsc_init() {
    uint8_t old_rate = rtc_set_rate(RTC_RATE_256HZ);
    rtc_calibrate(tsc_calibrate);

    while(!_tsc_ticks_per_ns);

    rtc_set_rate(old_rate);

    info("timestamp counter calibrated to %ld ticks per nanosecond\n", _tsc_ticks_per_ns);
}

INSTALL_EXTENSION(EXTP_TIMER_INIT, tsc_init, "timestamp counter");

