/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include <tachyon.h>
#include <log.h>
#include <ksym.h>

void abort(void) {
    error("out of luck - aborted.\n");

    list_t* trace = ksym_trace();
    ksym_write_trace(Error, trace);
    list_delete(trace);

    stop:
        asm("cli; hlt;");
        goto stop;
}
