/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
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

void ksym_write_trace(log_level_t level, list_t* trace) {
    list_node_t* node = list_begin(trace);

    while(node) {
        ksym_t* current = (ksym_t*)node->data;

        log_write(level, "\t[%p] %s\n", 
            current ? current->addr : 0x0, 
            current ? current->name : "<unknown>");

        node = node->next;
    }
}

list_t* ksym_trace() {
    list_t* trace = list_new();
    register uintptr_t* basep = ksym_get_bp();

    while(basep && basep[0] && basep[1]) {
        ksym_t const* current = ksym_get((void*)basep[1]);
        list_add(trace, current);
        basep = (uintptr_t*)basep[0];
    }

    return trace;
}
