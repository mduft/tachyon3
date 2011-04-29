/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#define CORE_LMA_X86        0x00100000
#define CORE_VMA_X86        0xC0000000

#define KHEAP_START         0xC1000000
#define KHEAP_END           0xE0000000

#define APIC_VIRTUAL        0xE0000000
#define APIC_VSZ            0x1000

#define CGA_VIRTUAL         (CORE_VMA_X86 - 0x1000)
#define CGA_VSZ             0x1000

#define RM_VIRTUAL          APIC_VIRTUAL + APIC_VSZ
#define RM_VSZ              0x110000

/* ksym support */
#define INTR_MAGIC_FRAME    0xDEADBEEF

/* registers */
#define AX  "eax"
#define BX  "ebx"
#define CX  "ecx"
#define DX  "edx"
