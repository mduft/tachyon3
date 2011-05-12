# Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
# This file is part of the 'tachyon' operating system.

#include <x86/defs.h>
#include <x86/asm.h>
#include <x86/gdt.h>

SECTION_BOOT_DATA

.global x86_gdt_code
.global x86_gdt_data
.global x86_gdt_ptr
.global x86_tss

# .----------------------------------------.
# | 32 kernel gdt code and data segments   |
# '----------------------------------------'

x86_gdt:
    .quad 0x0

    x86_gdt_code:
        .long 0x0000FFFF
        .long 0x000F0000 | GDT_SFL_CODE32
    x86_gdt_data:
        .long 0x0000FFFF
        .long 0x000F0000 | GDT_SFL_DATA32

    x86_tss:
        .quad 0x0

# .----------------------------------------.
# | 32 kernel gdt pointer (limit + ptr)    |
# '----------------------------------------'

x86_gdt_ptr:
    .word . - x86_gdt - 1
    .long x86_gdt

