# Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
# This file is part of the 'tachyon' operating system.

#include <x86_64/defs.h>
#include <x86/asm.h>
#include <x86/paging.h>

SECTION_BOOT_DATA

.global x86_64_pg_pml4
.global x86_64_pg_tmap
.global x86_64_pg_kernel_pdpt
.global x86_64_pg_pt_low

#define PG_BASE_KFLAGS (PG_PRESENT | PG_WRITABLE)

# .----------------------------------------.
# | PT's, currently only for low memory,   |
# | taking care of not mapping NULL, and   |
# | identity mapping the rest.             |
# '----------------------------------------'

.macro x86_64_pte cnt addr
.if \cnt
    .quad \addr + (PG_BASE_KFLAGS | PG_GLOBAL)
    x86_64_pte "(\cnt - 1)", "(\addr + 0x1000)"
.endif
.endm

.align 0x1000
x86_64_pg_pt_low:
    .quad 0x0
    x86_64_pte 0x64, 0x00001000
    x86_64_pte 0x64, 0x00065000
    x86_64_pte 0x64, 0x000C9000
    x86_64_pte 0x64, 0x0012D000
    x86_64_pte 0x64, 0x00191000
    x86_64_pte 0x0B, 0x001F5000

x86_64_pg_pt_temp:
    .fill 0x1F6, 0x8, 0x0   # -2MB - -40KB
    x86_64_pg_tmap:
    .fill 0xa, 0x8, 0x0     # -40KB - -0KB

# .----------------------------------------.
# | PD's, mapping 2MB pages, except low1,  |
# | which needs to make sure that the low  |
# | 4KB (NULL!) are unmapped!              |
# '----------------------------------------'

.align 0x1000
x86_64_pg_pd_low:
    .quad x86_64_pg_pt_low + ( PG_BASE_KFLAGS )               # 0 - 2MB
    .quad 0x200000         + ( PG_BASE_KFLAGS | PG_LARGE | PG_GLOBAL )   # 2 - 4MB
    .fill 0x1FE, 0x8, 0x0 # 510 not present entries.          # 4MB - 1GB unmap.

.align 0x1000
x86_64_pg_pd_high:
    .quad 0x000000        + ( PG_BASE_KFLAGS | PG_LARGE | PG_GLOBAL )   # 0 - 2MB (-2GB)
    .quad 0x200000        + ( PG_BASE_KFLAGS | PG_LARGE | PG_GLOBAL )   # 2 - 4MB (-2GB)
    .fill 0x1FE, 0x8, 0x0                                     # 4MB - -1GB unm.

.align 0x1000
x86_64_pg_pd_temp:
    .fill 0x1FF, 0x8, 0x0                            # -1GB - -2MB
    .quad x86_64_pg_pt_temp + ( PG_BASE_KFLAGS )     # -2MB - -0MB

# .----------------------------------------.
# | PDPT's, pointing to PD's               |
# '----------------------------------------'

.align 0x1000
x86_64_pg_pdpt_low:
    .quad x86_64_pg_pd_low + ( PG_BASE_KFLAGS )       # 0 - 1GB
    .fill 0x1FF, 0x8, 0x0                             # 1GB - 512GB unmap.

.align 0x1000
x86_64_pg_kernel_pdpt:
    .fill 0x1FE, 0x8, 0x0                             # -512GB - -2GB unmap.
    .quad x86_64_pg_pd_high + ( PG_BASE_KFLAGS )      # -2GB - -1GB
    .quad x86_64_pg_pd_temp + ( PG_BASE_KFLAGS )      # -1GB - -0GB
    

# .----------------------------------------.
# | The actual PML4, pointing to PDPT's    |
# '----------------------------------------'

.align 0x1000
x86_64_pg_pml4:
    .quad x86_64_pg_pdpt_low + ( PG_BASE_KFLAGS )     # 0 - 512GB
    .fill 0x1FE, 0x8, 0x0                             # 509 zero entries.
    .quad x86_64_pg_kernel_pdpt + ( PG_BASE_KFLAGS )  # -512GB - -0GB
