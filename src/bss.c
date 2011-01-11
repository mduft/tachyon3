/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "tachyon.h"

extern intptr_t _core_vma_edata;
extern intptr_t _core_vma_ebss;

/**
 * Clears the .bss section in memory to all zeros. This is normally
 * done by the bootloader, if everything goes well. Still, if we're
 * loaded directly from a PXE ROM, or in other cases, this still
 * helps a lot.
 */
void bss_init() {
    intptr_t* pstart = &_core_vma_edata;
    intptr_t* pend =   &_core_vma_ebss;

    while(pstart < pend) {
        *pstart++ = 0x0;
    }
}
