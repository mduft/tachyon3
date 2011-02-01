/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "tachyon.h"
#include "pmem.h"
#include "log.h"
#include "ldsym.h"
#include "mboot.h"
#include "spl.h"
#include "rm.h"

init_state_t const boot_state;

void boot() {
    log_init();
    log_set_level("screen", Trace);

    pmem_init();

    /* reserve the kernel's physical memory, so nobody
     * else tries to use it */
    if(!pmem_reserve(0xA0000, (((size_t)&_core_lma_end) - 0xA0000))) {
        error("failed to protect physical lower and kernel memory\n");
    }

    fatal("kernel ended unexpectedly.\n");
}
