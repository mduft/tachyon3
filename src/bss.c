/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "tachyon.h"

extern intptr_t _core_vma_edata;
extern intptr_t _core_vma_ebss;

void bss_init() {
    intptr_t* pstart = &_core_vma_edata;
    intptr_t* pend =   &_core_vma_ebss;

    while(pstart < pend) {
        *pstart++ = 0x0;
    }
}
