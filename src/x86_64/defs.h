/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

/* attention: virtual address space is divided in two halfs
 * (see: canonical addresses)
 *
 * with 40-bits, this means:
 *  lower half:  0x0000000000000000 - 0x0000007FFFFFFFFF
 *  higher half: 0xFFFFFF8000000000 - 0xFFFFFFFFFFFFFFFF
 *
 * with 48-bits, it would be
 *  lower half:  0x0000000000000000 - 0x00007FFFFFFFFFFF
 *  higher half: 0xFFFF800000000000 - 0xFFFFFFFFFFFFFFFF
 *
 * with 56-bits, it would be
 *  lower half:  0x0000000000000000 - 0x007FFFFFFFFFFFFF
 *  higher half: 0xFF80000000000000 - 0xFFFFFFFFFFFFFFFF
 *
 * additionally, the gcc kernel memory model assumes the kernel
 * to be above 0xFFFFFFFF80000000 !!
 */

#define CORE_LMA_X86_64     0x0000000000100000
#define CORE_VMA_X86_64     0xFFFFFFFF80000000
#define CORE_VMA_X86_64_END (CORE_VMA_X86_64 + 0x1000000)

#define KHEAP_START         0xFFFFFF0000000000
#define KHEAP_END           0xFFFFFFFF70000000

#define KSHEAP_START        0xFFFFFFFF70000000
#define KSHEAP_END          0xFFFFFFFF80000000

#define PHEAP_START         0x0000000000400000
#define PHEAP_END           0x0000700000000000

#define SHEAP_START         0x0000700000000000
#define SHEAP_END           0x0000800000000000

#define APIC_VIRTUAL        (CORE_VMA_X86_64_END)
#define APIC_VSZ            0x1000

#define CGA_VIRTUAL         (APIC_VIRTUAL + APIC_VSZ)
#define CGA_VSZ             0x1000

#define RM_VIRTUAL          (CGA_VIRTUAL + CGA_VSZ)
#define RM_VSZ              0x110000

#define GDT_VIRTUAL         (RM_VIRTUAL + RM_VSZ)
#define GDT_VSZ             0x1000

/* ksym support */
#define INTR_MAGIC_FRAME    0x00900d900dc0ffee

/* rings. */
#define RING_KERNEL         0
#define RING_DRIVERS        3
#define RING_USERSPACE      3

/* registers */
#define AX  "rax"
#define BX  "rbx"
#define CX  "rcx"
#define DX  "rdx"
