/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "tachyon.h"
#include "ldsym.h"

/**
 * Clears the .bss section in memory to all zeros. This is normally
 * done by the bootloader, if everything goes well. Still, if we're
 * loaded directly from a PXE ROM, or in other cases, this still
 * helps a lot.
 */
void bss_init() {
    uintptr_t* pstart = &_core_vma_bss;
    uintptr_t* pend =   &_core_vma_ebss;

    while(pstart < pend) {
        *pstart++ = 0x0;
    }
}
