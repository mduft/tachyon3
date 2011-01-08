/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "tachyon.h"

void abort(void) {

    stop:
        asm("cli; hlt;");
        goto stop;
}
