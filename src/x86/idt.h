/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#define IDTT_SMALL_TSS_AVAIL    (0x1 << 8)
#define IDTT_LDT                (0x2 << 8)
#define IDTT_SMALL_TSS_BUSY     (0x3 << 8)
#define IDTT_SMALL_CALL_GATE    (0x4 << 8)
#define IDTT_TASK_GATE          (0x5 << 8)
#define IDTT_SMALL_INT_GATE     (0x6 << 8)
#define IDTT_SMALL_TRAP_GATE    (0x7 << 8)
#define IDTT_LARGE_TSS_AVAIL    (0x9 << 8)
#define IDTT_LARGE_TSS_BUSY     (0xb << 8)
#define IDTT_LARGE_CALL_GATE    (0xc << 8)
#define IDTT_LARGE_INT_GATE     (0xe << 8)
#define IDTT_LARGE_TRAP_GATE    (0xf << 8)

#define IDTE_PRESENT            (0x1 << 15)

#define EX_DIV_ERR     0
#define EX_DEBUG_EX    1
#define EX_NMI         2
#define EX_BREAKPOINT  3
#define EX_OVERFLOW    4
#define EX_BOUND_RANGE 5
#define EX_INVALID_OP  6
#define EX_DEVICE_NA   7
#define EX_DFAULT      8
#define EX_CO_SEG_OF   9
#define EX_INVALID_TSS 10
#define EX_SEG_NP      11
#define EX_STACK_FAULT 12
#define EX_GPF         13
#define EX_PAGE_FAULT  14
#define EX_FPE         16
#define EX_ALIGN_CHECK 17
#define EX_MCE         18
#define EX_SIMD_FPE    19

