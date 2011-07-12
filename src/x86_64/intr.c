/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include <tachyon.h>

#include <x86/reg.h>
#include "cpu.h"

bool intr_state() {
    register uint64_t rflags = 0;

    asm volatile("pushf; popq %0" : "=r"(rflags));

    return (rflags & FL_IF);
}

void intr_disable() {
    asm volatile("cli");
    cpu_locals_t* lcls = cpu_locals(cpu_current_id());
    lcls->ifda_cnt++;
}

void intr_enable() {
    cpu_locals_t* lcls = cpu_locals(cpu_current_id());

    if(lcls->ifda_cnt > 0)
        lcls->ifda_cnt--;

    if(lcls->ifda_cnt == 0)
        asm volatile("sti");
}

