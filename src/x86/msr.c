/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "msr.h"

uint64_t msr_read(uint32_t msr) {
    register uint32_t v_low, v_high;

    asm volatile("rdmsr" : "=d"(v_high), "=a"(v_low) : "c"(msr));

    return (v_high << 31) | v_low;
}

void msr_write(uint32_t msr, uint64_t value) {
    asm volatile("wrmsr" :: "c"(msr), "d"((uintptr_t)(value >> 31)), "a"((uintptr_t)value));
}
