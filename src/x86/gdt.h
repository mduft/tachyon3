/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#define GDT_TYPE_CODE_OR_DATA   (1 << 12)
#define GDT_DPL_1               (1 << 13)
#define GDT_DPL_2               (2 << 13)
#define GDT_DPL_3               (3 << 13)
#define GDT_PRESENT             (1 << 15)
#define GDT_64BIT               (1 << 21)
#define GDT_DEF_OPSIZE_32BIT    (1 << 22)
#define GDT_GRANULARITY         (1 << 23)

#define GDT_TYPE_CODE           (1 << 11)
#define GDT_DATA_EXPAND_DOWN    (1 << 10)
#define GDT_DATA_WRITE_ENABLE   (1 << 9)

#define GDT_ACCESSED            (1 << 8)

#define GDT_CODE_CONFORMING     (1 << 10)
#define GDT_CODE_READ_ENABLE    (1 << 9)

/* gate indices */

#define GDT_KCODE32  0x08
#define GDT_KDATA32  0x10
#define GDT_KCODE64  0x18
#define GDT_KDATA64  0x20

