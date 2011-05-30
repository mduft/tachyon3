/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "tachyon.h"
#include "extp.h"
#include "log.h"
#include "tsrc.h"

static void tsrc_init() {
    // find all timesource extensions, and choose one of them!
    // maybe make this configurable somehow? kernel command line?
    // config file?
}

INSTALL_EXTENSION(EXTP_KINIT, tsrc_init, "time source");

bool tsrc_schedule(tsrc_cb_t callback, ssize_t ms, bool oneshot) {
    NOT_IMPLEMENTED(__func__);
    return false;
}

