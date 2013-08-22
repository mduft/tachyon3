/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

/** 
 * @mainpage X86-64 Tachyon
 * current kernel fixed virtual memory map is as follows:
 *
 * <pre>
 * 
 * 0x0                     0x800000000000             0xFFFFFFFF70000000
 *     0x100000   0x700000000000           0xFFFFFF0000000000  0xFFFFFFFF80000000
 * .-----------------------------------------------------------------------------
 * | F | PHEAP    | PSHEAP | ... resvd ... | KHEAP    | KSHEAP | CORE        ...
 * '-----------------------------------------------------------------------------
 *
 *                0xFFFFFFFF81001000  0xFFFFFFFF81112000   0xFFFFFFFF81123000
 *      0xFFFFFFFF81000000  0xFFFFFFFF81002000  0xFFFFFFFF81113000
 * --------------------------------------------------------.
 *  ... | APIC    | CGA     | RME-MEM | GDT-TMP | UAPI ... |
 * --------------------------------------------------------'
 *
 * </pre>
 *
 * This make following virtual sizes for dynamic regions:
 *
 * * for each process seperately:
 *  * PHEAP (per-process heap):   123145301262336 bytes (~112 TB)
 *  * PSHEAP(per-process stacks):  17592186044416 bytes (~ 16 TB)
 *
 * * shared accross all processes (kernel memory):
 *  * KHEAP (kernel heap):          1097095708672 bytes (~  1 TB)
 *  * KSHEAP (kernel stacks):           268435456 bytes (~256 MB)
 */

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

/** the kernels physical load address */
#define CORE_LMA_X86_64     0x0000000000100000
/** the kernels virtual higher half start address */
#define CORE_VMA_X86_64     0xFFFFFFFF80000000
/** the kernels virtual higher half end (+safety region) */
#define CORE_VMA_X86_64_END (CORE_VMA_X86_64 + 0x1000000)

/** the kernel heap start */
#define KHEAP_START         0xFFFFFF0000000000
/** the kernel heap end */
#define KHEAP_END           0xFFFFFFFF70000000

/** the kernel stack heap region start */
#define KSHEAP_START        0xFFFFFFFF70000000
/** the kernel stack heap region end */
#define KSHEAP_END          0xFFFFFFFF80000000

/** the per-process heap region start */
#define PHEAP_START         0x0000000000400000
/** the per-process heap region end */
#define PHEAP_END           0x0000700000000000

/** the per-process stack heap region start */
#define SHEAP_START         0x0000700000000000
/** the per-process stack heap region end */
#define SHEAP_END           0x0000800000000000

/** virtual location of the current cpu's local APIC */
#define APIC_VIRTUAL        (CORE_VMA_X86_64_END)
/** size of the region reserved at the APIC location */
#define APIC_VSZ            0x1000

#define IOAPIC_VIRTUAL      (APIC_VIRTUAL + APIC_VSZ)
#define IOAPIC_VSZ          0x1000
#define IOAPIC_MAX_CNT      0x10

/** virtual location of the CGA screen buffer */
#define CGA_VIRTUAL         (IOAPIC_VIRTUAL + (IOAPIC_VSZ * IOAPIC_MAX_CNT))
/** size of the region reserved at the CGA location */
#define CGA_VSZ             0x1000

/** virtual location of the real mode memory (mapped to physical 0x0 and up) */
#define RM_VIRTUAL          (CGA_VIRTUAL + CGA_VSZ)
/** size of the real mode emulator memory block */
#define RM_VSZ              0x110000

/** virtual region reserved for temporarily mapping a GDT during cpu setup */
#define GDT_VIRTUAL         (RM_VIRTUAL + RM_VSZ)
/** size of the temporary GDT region */
#define GDT_VSZ             0x1000

/** virtual starting address of the UAPI. */
#define UAPI_VMA_X86_64     (GDT_VIRTUAL + GDT_VSZ)

/** maximum size for the mapped uapi code */
#define UAPI_MAX_SZ         0x10000

/** ksym support: magic stack frame cookie */
#define INTR_MAGIC_FRAME    0x00900d900dc0ffee

/* rings. */
#define RING_KERNEL         0
#define RING_DRIVERS        2
#define RING_USERSPACE      3

/* registers */
#define AX  "rax"
#define BX  "rbx"
#define CX  "rcx"
#define DX  "rdx"
