/* Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
 * This file is part of the 'tachyon' operating system. */

#include "cmd.h"
#include "extp.h"
#include "string.h"
#include "log.h"
#include "mem.h"
#include "kheap.h"

static char const* _cmdline[MAX_CMD_ARGUMENTS];

static void cmd_extp_handler(char const* tag, extp_func_t cb, char const* descr) {
    if(cb) cb();
}

void cmd_init() {
    // clear out the array
    memset(_cmdline, 0, sizeof(_cmdline));

    // allow extensions to contribute.
    extp_iterate(EXTP_CMD_LINE, cmd_extp_handler);
}

void cmd_add(char const* argument) {
    // make sure the argument is stored in some kernel memory - we will never free this, that's ok.
    if(!argument) {
        warn("NULL argument for command line skipped\n");
        return;
    }

    debug("adding kernel cmd: '%s'\n", argument);

    char * target = kheap_alloc(strlen(argument) * sizeof(char));

    // skip to the first null element in the command line, and set it.
    char const** current = _cmdline;
    while(*current) {
        current++;
    }

    *current = target;
}

char const** cmd_get() {
    return _cmdline;
}

