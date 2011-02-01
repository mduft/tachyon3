/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "rm.h"
#include "pmem.h"
#include <contrib/rme2/rme.h>

void rm_init() {
    /* TODO: allocate virtual memory, and map NULL
     * - (NULL + RME_BLOCK_SIZE) there. */

    pmem_reserve(0, RME_BLOCK_SIZE);
}

bool rm_int(uint8_t vec, rm_state_t* state) {
    return false;
}

bool rm_call(rm_state_t* state) {
    return false;
}
