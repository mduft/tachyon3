/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include <x86/tsc.h>

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

