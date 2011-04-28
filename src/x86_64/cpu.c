/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "cpu.h"
#include "thread.h"
#include <extp.h>
#include <kheap.h>
#include <mem.h>

void cpu_init() {
    x86_64_ctx_set(kheap_alloc(sizeof(thr_context_t)));
}

INSTALL_EXTENSION(EXTP_CPUINIT, cpu_init, "cpu context");

