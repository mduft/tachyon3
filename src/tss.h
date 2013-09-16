/* copyright (c) 2011 by markus duft <mduft@gentoo.org>
 * this file is part of the 'tachyon' operating system. */

#pragma once

#include <tachyon.h>

#define IST_STACK_SIZE      0x1000

/**
 * Describes a TSS, pointed to by a system selector in the
 * dyamic GDT.
 */
typedef struct {
    uint32_t _res0;
    uint64_t rsp[3];            /**< array of system stack pointers for all rings. */
    uint64_t ist[8];            /**< interrupt stacks. 0 is reserved. */
    uint64_t _res1;
    uint16_t _res2;
    uint16_t iobm_off;          /**< I/O permission bitmap offset from actual tss location. */
} PACKED tss_t;

void tss_init();
