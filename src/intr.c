/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "tachyon.h"
#include "log.h"
#include "intr.h"
#include "list.h"

/** 
 * the maximum supported interrupt gate. this is fixed
 * in <arch>/idt.S by initializing those gates.
 */
#define MAX_INTR    60

/**
 * Describes an entry in the interrupt handler tabel.
 */
typedef struct {
    bool list;  /**< determines whether the entry is a single handler or a list of handlers. */
    union {
        intr_handler_t handler; /**< the single handler. */
        list_t* list;           /**< the list of handlers. */
    } h;
} intr_gate_desc_t;

/**
 * Holds the desciptors of all interrupt gates.
 */
intr_gate_desc_t intr_gate_table[MAX_INTR];

/**
 * C lowest-level iterrupt handler. All interrupts are routed here
 * from the assembly stubs/trampolines.
 *
 * @param state state information for the interrupt.
 */
void intr_dispatch(interrupt_t* state) {
    if(state->num >= MAX_INTR)
        fatal("cannot handle out of bounds gate: %d at %p\n", state->num, state->ip);

    intr_gate_desc_t* gate = &intr_gate_table[state->num];

    if(gate->list) {
        list_t* handlers = gate->h.list;
        list_node_t* node = list_begin(handlers);

        while(node) {
            intr_handler_t handler = node->data;

            if(handler && handler(state))
                return;

            node = node->next;
        }
        
    } else {
        intr_handler_t handler = gate->h.handler;

        if(handler && handler(state))
            return;
    }

    fatal("unhandled interrupt %d in %p\n", state->num, state->ip);
}

void intr_add(uint16_t num, intr_handler_t handler) {
    if(num >= MAX_INTR)
        fatal("gate not available!\n");

    intr_gate_desc_t* gate = &intr_gate_table[num];

    if(gate->list) {
        list_t* list = gate->h.list;
        if(!list) {
            list = list_new();
            gate->h.list = list;
        }
        list_add(list, handler);
        trace("adding handler for %d: %p (new handler count: %d)\n", num, handler, list_size(list));
    } else {
        trace("setting handler for %d: %p (old: %p)\n", num, handler, gate->h.handler);
        gate->h.handler = handler;
    }
}

void intr_remove(uint16_t num, intr_handler_t handler) {
    if(num >= MAX_INTR)
        fatal("gate not available!\n");

    intr_gate_desc_t* gate = &intr_gate_table[num];

    if(gate->list) {
        list_t* list = gate->h.list;
        if(list) {
            list_remove(list, handler);
            trace("removing handler for %d: %p (new handler count: %d\n", num, handler, list_size(list));
        }
    } else {
        if(gate->h.handler != handler)
            fatal("trying to remove foreign handler for %d: %p != %p\n", num, gate->h.handler, handler);

        gate->h.handler = NULL;
    }
}
