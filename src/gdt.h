/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#define GDT_SH_TYPE_CODE_OR_DATA    (12)
#define GDT_SH_DPL                  (13)
#define GDT_SH_PRESENT              (15)
#define GDT_SH_64BIT                (21)
#define GDT_SH_32BIT                (22)
#define GDT_SH_GRANULARITY          (23)
#define GDT_SH_TYPE                 (8)

#define GDT_TYPE_ACCESSED           0x1

#define GDT_TYPE_CODE               0x8
#define GDT_TYPE_CODE_READ_ENABLE   0x2
#define GDT_TYPE_CODE_CONFORMING    0x4

#define GDT_TYPE_DATA_WRITE_ENABLE  0x2
#define GDT_TYPE_DATA_EXPAND_DOWN   0x4

#define GDT_TYPE_SYSTEM             0x8
#define GDT_TYPE_TSS                (GDT_TYPE_SYSTEM | 0x1)

/* predefined selector flags */
#define GDT_SFL_BASE \
    (   (1 << GDT_SH_TYPE_CODE_OR_DATA) \
    |   (1 << GDT_SH_GRANULARITY) \
    |   (1 << GDT_SH_PRESENT) )

#define GDT_SFL_CODE \
    (   GDT_SFL_BASE \
    |   ((GDT_TYPE_CODE | GDT_TYPE_CODE_READ_ENABLE) << GDT_SH_TYPE) )

#define GDT_SFL_DATA \
    (   GDT_SFL_BASE \
    |   ((GDT_TYPE_DATA_WRITE_ENABLE) << GDT_SH_TYPE) )

#define GDT_SFL_CODE32  GDT_SFL_CODE | (1 << GDT_SH_32BIT)
#define GDT_SFL_CODE64  GDT_SFL_CODE | (1 << GDT_SH_64BIT)
#define GDT_SFL_DATA32  GDT_SFL_DATA | (1 << GDT_SH_32BIT)
#define GDT_SFL_DATA64  GDT_SFL_DATA | (1 << GDT_SH_64BIT)

/* gate indices */

#define GDT_KCODE32  0x08
#define GDT_KDATA32  0x10
#define GDT_KCODE64  0x18
#define GDT_KDATA64  0x20
#define GDT_KTSS     0x28 /* attention: req. 2 slots! */

#define GDT_UCODE64  0x38
#define GDT_UDATA64  0x40

