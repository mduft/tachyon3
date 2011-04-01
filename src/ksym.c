/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "ksym.h"
#include "log.h"

// this is an array, but flat.
extern ksym_t const ksym_table;

ksym_t const* ksym_get(void* addr) {
    ksym_t const* p = &ksym_table;

    while(p && p->addr) {
        if((uintptr_t)addr >= p->addr && (uintptr_t)addr <= (p->addr + p->size))
            return p;

        p++;
    }

    return NULL;
}

void ksym_trace(log_level_t level) {
    register uintptr_t* basep = ksym_get_bp();

    while(basep && basep[0] && basep[1]) {
        ksym_t const* current = ksym_get((void*)basep[1]);

        log_write(level, "\t[%p] %s+%d\n", basep[1], 
            current ? current->name : "<unknown>", 
            current ? basep[1] - current->addr : 0);

        basep = (uintptr_t*)basep[0];
    }
}
