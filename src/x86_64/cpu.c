/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "cpu.h"
#include <extp.h>
#include <kheap.h>
#include <mem.h>

extern void x86_64_ctx_init();

/* TODO: FIXME: one per cpu, need GDT entry for %gs */
cpu_context_t the_one_and_only_context;

void cpu_init() {
    memset(&the_one_and_only_context, 0, sizeof(cpu_context_t));
}

INSTALL_EXTENSION(EXTP_CPUINIT, cpu_init, "cpu context");

