/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "tachyon.h"
#include "pmem.h"
#include "log.h"
#include "ldsym.h"
#include "mboot.h"

/* testing */
#if 1
#include "kheap.h"
#endif

init_state_t const boot_state;

void boot() {
    log_init();
    log_set_level("screen", Trace);

    mboot_init();

    pmem_init();

    /* reserve the kernel's physical memory, so nobody
     * else tries to use it */
    if(!pmem_reserve(0xA0000, (((size_t)&_core_lma_end) - 0xA0000))) {
        error("failed to protect physical lower and kernel memory\n");
    }

    void* p1 = kheap_alloc(0x10);
    void* p2 = kheap_alloc(0x20);
    void* p3 = kheap_alloc(0x3);

    kheap_free(p1);
    kheap_free(p2);
    kheap_free(p3);
}
