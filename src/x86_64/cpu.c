/* copyright (c) 2011 by markus duft <mduft@gentoo.org>
 * this file is part of the 'tachyon' operating system. */

#include "cpu.h"
#include "thread.h"
#include "dyngdt.h"
#include "tss.h"

#include <extp.h>
#include <kheap.h>
#include <mem.h>

#include <x86/gdt.h>

void cpu_init() {
    /* create and set an initial dummy context, which will be 
       released on the first context switch. */
    x86_64_ctx_set(kheap_alloc(sizeof(thr_context_t)));

    dyngdt_init_and_lock();

    dyngdt_set(GDT_KCODE64, 0, 0xFFFFFFFF, GDT_TYPE_CODE | GDT_TYPE_CODE_READ_ENABLE, 0, true, false);
    dyngdt_set(GDT_KDATA64, 0, 0xFFFFFFFF, GDT_TYPE_DATA_WRITE_ENABLE, 0, true, false);

    tss_init();

    dyngdt_activate_and_unlock();

    /* now set the IST bits in the IDT */
}

INSTALL_EXTENSION(EXTP_CPUINIT, cpu_init, "cpu state");

