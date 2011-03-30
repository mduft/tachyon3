/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#define PG_PRESENT            (1 << 0)
#define PG_WRITABLE           (1 << 1)
#define PG_USER               (1 << 2)
#define PG_WRITETHROUGH       (1 << 3)
#define PG_NONCACHABLE        (1 << 4)
#define PG_ACCESSED           (1 << 5)
#define PG_DIRTY              (1 << 6)
#define PG_LARGE              (1 << 7)
#define PG_GLOBAL             (1 << 8)
#define PG_EXECUTE_DISABLE    (1 << 63)

#define PAGE_SIZE_4K            0x001000
#define PAGE_SIZE_2M            0x200000

#define PG_KFLAGS ( PG_PRESENT | PG_WRITABLE | PG_GLOBAL )

