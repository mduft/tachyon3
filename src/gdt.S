# Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
# This file is part of the 'tachyon' operating system.

#include <defs.h>
#include <asm.h>
#include <gdt.h>

SECTION_BOOT_DATA

.global x86_64_gdt_code
.global x86_64_gdt_data
.global x86_64_gdt_ptr

# .----------------------------------------.
# | 64 kernel gdt code and data segments   |
# | This GDT only has to contain segments  |
# | required before dyngdt kicks in        |
# '----------------------------------------'

.align 0x10
x86_64_gdt:
    .quad 0x0

    x86_64_gdt_code32:
        .long 0x0000FFFF
        .long 0x000F0000 | GDT_SFL_CODE32

    x86_64_gdt_data32:
        .long 0x0000FFFF
        .long 0x000F0000 | GDT_SFL_DATA32

    x86_64_gdt_code:
        .long 0x0000FFFF
        .long 0x000F0000 | GDT_SFL_CODE64
    x86_64_gdt_data:
        .long 0x0000FFFF
        .long 0x000F0000 | GDT_SFL_DATA64

# .----------------------------------------.
# | 64 kernel gdt pointer (limit + ptr)    |
# '----------------------------------------'

x86_64_gdt_ptr:
    .word . - x86_64_gdt - 1
    .quad x86_64_gdt

