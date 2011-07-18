/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "ksym.h"
#include "log.h"
#include "kheap.h"

#define KSYM_TRACE_MAX_LEN  256

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
        ksym_node_t* snode = (ksym_node_t*)node->data;

        if(!snode->sym) {
            log_write(level, "\t[%p] <unknown>\n", snode->real_addr);
        } else {
            if(snode->sym->addr == INTR_MAGIC_FRAME) {
                if(sizeof(uintptr_t) == 4)
                    log_write(level, "\t[-- intr --]\n");
                else if(sizeof(uintptr_t) == 8)
                    log_write(level, "\t[------ intr ------]\n");
            } else {
                log_write(level, "\t[%p] %s + 0x%x\n", 
                    snode->real_addr, 
                    snode->sym ? snode->sym->name : "<unknown>", 
                    snode->sym ? (snode->real_addr - snode->sym->addr) : 0);
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
        ksym_node_t* node = kheap_alloc(sizeof(ksym_node_t));

        if(basep[1] == INTR_MAGIC_FRAME) {
            ksym_node_t* ifnode = kheap_alloc(sizeof(ksym_node_t));
            ifnode->sym = &intr_magic_frame;
            ifnode->real_addr = 0;
            list_add(trace, ifnode);
            node->real_addr = ((void*)basep[2]);
        } else {
            node->real_addr = ((void*)basep[1]);
        }

        node->sym = ksym_get(node->real_addr);
        list_add(trace, node);

        basep = (uintptr_t*)basep[0];

        if(list_size(trace) >= KSYM_TRACE_MAX_LEN) {
            warn("trace too long, limiting\n");
            return trace;
        }
    }

    return trace;
}

void ksym_delete(list_t* trace) {
    list_node_t* node = list_begin(trace);

    while(node) {
        ksym_node_t* n = (ksym_node_t*)node->data;

        if(n)
            kheap_free(n);

        node = node->next;
    }

    list_delete(trace);
}
