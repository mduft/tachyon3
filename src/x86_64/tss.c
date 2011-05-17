/* copyright (c) 2011 by markus duft <mduft@gentoo.org>
 * this file is part of the 'tachyon' operating system. */

#include "tss.h"
#include "dyngdt.h"

#include <kheap.h>
#include <mem.h>

#include <x86/gdt.h>

void tss_init() {
    tss_t* tss = kheap_alloc(sizeof(tss_t));
    memset(tss, 0, sizeof(tss_t));

    // allocate IST stacks, ring stacks...
    tss->ist[IST_FAULT_STACK] = (uint64_t)kheap_alloc(IST_STACK_SIZE) + IST_STACK_SIZE;

    dyngdt_set(GDT_KTSS, (uintptr_t)tss, sizeof(tss_t), GDT_TYPE_TSS, 0, true, true);
}
