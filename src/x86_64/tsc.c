/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include <x86/tsc.h>
#include <x86/rtc.h>

#include <extp.h>
#include <log.h>
#include <intr.h>

#define CALIBRATE_TURNS 64

static volatile uint64_t _tsc_ticks_per_ns = 0;

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

