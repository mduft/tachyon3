/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "spl.h"

void spl_init(spinlock_t* lock) {
    lock->locked = 0;
}

