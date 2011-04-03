# Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
# This file is part of the 'tachyon' operating system.

#include <x86_64/defs.h>
#include <x86/asm.h>
#include <x86/gdt.h>

SECTION_BOOT_DATA

.global x86_64_gdt_code
.global x86_64_gdt_data
.global x86_64_gdt_ptr
.global x86_64_tss

# .----------------------------------------.
# | 64 kernel gdt code and data segments   |
# '----------------------------------------'

x86_64_gdt:
    .quad 0x0

    x86_64_gdt_code32:
        .long 0x0000FFFF
        .long 0x000F0000 | (GDT_PRESENT | GDT_TYPE_CODE_OR_DATA | GDT_TYPE_CODE | GDT_CODE_READ_ENABLE | GDT_DEF_OPSIZE_32BIT | GDT_GRANULARITY)

    x86_64_gdt_data32:
        .long 0x0000FFFF
        .long 0x000F0000 | (GDT_PRESENT | GDT_TYPE_CODE_OR_DATA | GDT_DATA_WRITE_ENABLE | GDT_DEF_OPSIZE_32BIT | GDT_GRANULARITY)

    x86_64_gdt_code:
        .long 0x0000FFFF
        .long 0x000F0000 | (GDT_64BIT | GDT_PRESENT | GDT_TYPE_CODE_OR_DATA | GDT_TYPE_CODE | GDT_CODE_READ_ENABLE | GDT_GRANULARITY)
    x86_64_gdt_data:
        .long 0x0000FFFF
        .long 0x000F0000 | (GDT_64BIT | GDT_PRESENT | GDT_TYPE_CODE_OR_DATA | GDT_DATA_WRITE_ENABLE | GDT_GRANULARITY)

    x86_64_tss:
        .quad 0x0

# .----------------------------------------.
# | 64 kernel gdt pointer (limit + ptr)    |
# '----------------------------------------'

x86_64_gdt_ptr:
    .word . - x86_64_gdt - 1
    .quad x86_64_gdt

