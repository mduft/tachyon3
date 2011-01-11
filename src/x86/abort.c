/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include <tachyon.h>
#include <log.h>

void abort(void) {
    error("out of luck - aborted.");

    stop:
        asm("cli; hlt;");
        goto stop;
}
