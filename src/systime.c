/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "systime.h"
#include "extp.h"
#include "log.h"

static systime_desc_t const* the_timesource = NULL;

static void systime_extp_init(char const* tag, extp_func_t func, char const* descr) {
    systime_extp ext = (systime_extp)func;
    systime_desc_t const* desc = ext();

    trace("checking system timesource: %s\n", descr);

    if(the_timesource == NULL || the_timesource->accuracy < desc->accuracy)
        the_timesource = desc;
}

void systime_init() {
    extp_iterate(EXTP_SYSTIME, systime_extp_init);

    if(the_timesource == NULL || the_timesource->systime_us_func == NULL)
        fatal("no system time source found!\n");

    if(the_timesource->systime_init_func)
        the_timesource->systime_init_func();
}

uint64_t systime() {
    return the_timesource->systime_us_func();
}

void systime_stall(uint64_t us) {
    uint64_t start = systime();
    uint64_t end = start + us;

    while(systime() < end);
}
