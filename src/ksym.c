/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "ksym.h"

// this is an array, but flat.
extern ksym_t const ksym_table;

ksym_t const* ksym_get(void* addr) {
    ksym_t const* p = &ksym_table;

    while(p && p->addr) {
        if((uintptr_t)addr >= p->addr && (uintptr_t)addr < (p->addr + p->size))
            return p;

        p++;
    }

    return NULL;
}

