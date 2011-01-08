# Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
# This file is part of the 'tachyon' operating system.

#include <x86/defs.h>
#include <x86/asm.h>
#include <x86/paging.h>

SECTION_BOOT_DATA

.global x86_pg_pd
.global x86_temp_mapspace

#define PG_BASE_KFLAGS  ( PG_PRESENT | PG_WRITABLE | PG_GLOBAL )

# .----------------------------------------.
# | PT's, currently only for low memory,   |
# | taking care of not mapping NULL, and   |
# | identity mapping the rest.             |
# '----------------------------------------'

.macro x86_pte cnt addr
.if \cnt
    .long \addr + ( PG_BASE_KFLAGS )
    x86_pte "(\cnt - 1)", "(\addr + 0x1000)"
.endif
.endm

# 4KB - 4MB
.align 0x1000
x86_pg_pt:
    .long 0x0
    x86_pte 0x64, 0x00001000
    x86_pte 0x64, 0x00065000
    x86_pte 0x64, 0x000C9000
    x86_pte 0x64, 0x0012D000
    x86_pte 0x64, 0x00191000
    x86_pte 0x64, 0x001F5000
    x86_pte 0x64, 0x00259000
    x86_pte 0x64, 0x002BD000
    x86_pte 0x64, 0x00321000
    x86_pte 0x64, 0x00385000
    x86_pte 0x17, 0x003E9000

x86_pg_pt_temp:
    .fill 0x3F6, 0x4, 0x0
    x86_temp_mapspace:
    .fill 0xa, 0x4, 0x0                  # reserved -> temp mapping

# .----------------------------------------.
# | PD's, mapping 4MB pages, except low1,  |
# | which needs to make sure that the low  |
# | 4KB (NULL!) are unmapped!              |
# '----------------------------------------'

# 0MB - 4MB, rest (until 4GB) unmapped
.align 0x1000
x86_pg_pd:
    .long x86_pg_pt + ( PG_BASE_KFLAGS ) # 0 - 4MB
    .fill 0x2FF, 0x4, 0x0                # 767 not present entries.
    .long x86_pg_pt + ( PG_BASE_KFLAGS ) # higher half map
    .fill 0x0FE, 0x4, 0x0                # 254 not present entries.
    .long x86_pg_pt_temp + ( PG_BASE_KFLAGS ) # temp mapping


