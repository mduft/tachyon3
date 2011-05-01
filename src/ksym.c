/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "ksym.h"
#include "log.h"

// this is an array, but flat.
extern ksym_t const ksym_table;
static ksym_t const intr_magic_frame = {
    INTR_MAGIC_FRAME, 0, " -- interrupt"
};

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
    ksym_write_trace_top(level, trace, -1);
}

void ksym_write_trace_top(log_level_t level, list_t* trace, int16_t limit) {
    list_node_t* node = list_begin(trace);
    int16_t num = 0;

    while(node) {
        ksym_t* current = (ksym_t*)node->data;

        if(!current) {
            log_write(level, "\t[%p] <unknown>\n", 0);
        } else {
            if(current->addr == INTR_MAGIC_FRAME) {
                if(sizeof(uintptr_t) == 4)
                    log_write(level, "\t[-- intr --]\n");
                else if(sizeof(uintptr_t) == 8)
                    log_write(level, "\t[------ intr ------]\n");
            } else {
                log_write(level, "\t[%p] %s\n", 
                    current ? current->addr : 0x0, 
                    current ? current->name : "<unknown>");
            }
        }

        node = node->next;

        if(limit != -1 && ++num >= limit)
            break;
    }
}

list_t* ksym_trace() {
    list_t* trace = list_new();
    register uintptr_t* basep = ksym_get_bp();

    while(basep && basep[0] && basep[1]) {
        ksym_t const* current;
        if(basep[1] == INTR_MAGIC_FRAME) {
            list_add(trace, &intr_magic_frame);
            current = ksym_get((void*)basep[2]);
        } else {
            current = ksym_get((void*)basep[1]);
        }
        list_add(trace, current);
        basep = (uintptr_t*)basep[0];
    }

    return trace;
}
