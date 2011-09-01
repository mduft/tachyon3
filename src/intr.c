/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "tachyon.h"
#include "log.h"
#include "intr.h"
#include "list.h"
#include "reg.h"

/** 
 * the maximum supported interrupt gate. this is fixed
 * in <arch>/idt.S by initializing those gates.
 */
#define MAX_INTR    60

/**
 * Describes an entry in the interrupt handler tabel.
 */
typedef struct {
    gatemode_t mode;            /**< the gates trigger mode */
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
void intr_dispatch(interrupt_t* state, uint16_t num) {
    if(num >= MAX_INTR)
        fatal("gate %d not available\n", num);

    intr_gate_desc_t* gate = &intr_gate_table[num];

    if(gate->mode & GateModeMultiHandler) {
        list_t* handlers = gate->h.list;
        list_node_t* node = list_begin(handlers);

        while(node) {
            intr_handler_t handler = node->data;

            if(handler && handler(state) && !(gate->mode & GateModeNotifyAll))
                return;

            node = node->next;
        }
        
    } else {
        intr_handler_t handler = gate->h.handler;

        if(handler && handler(state))
            return;
    }

    fatal("unhandled interrupt %d @ %p\n", num, state->ip);
}

struct _tmp {
    uintptr_t rsp;
    uintptr_t rsi;  
    uintptr_t rdi;
};

void intr_do_test(interrupt_t* state, struct _tmp* tmp) {
    trace("%d: iretq to %p (stk at %p)\n", state->num, state->ip, tmp->rsp);
}

void intr_add(uint16_t num, intr_handler_t handler) {
    if(num >= MAX_INTR)
        fatal("gate %d not available\n", num);

    intr_gate_desc_t* gate = &intr_gate_table[num];

    if(gate->mode & GateModeMultiHandler) {
        list_t* list = gate->h.list;
        if(!list) {
            list = list_new();
            gate->h.list = list;
        }
        list_add(list, handler);
        trace("adding handler for interrupt %d: %p (new handler count: %d)\n", num, handler, list_size(list));
    } else {
        trace("setting handler for interrupt %d: %p (old: %p)\n", num, handler, gate->h.handler);
        gate->h.handler = handler;
    }
}

void intr_remove(uint16_t num, intr_handler_t handler) {
    if(num >= MAX_INTR)
        fatal("gate %d not available\n", num);

    intr_gate_desc_t* gate = &intr_gate_table[num];

    if(gate->mode & GateModeMultiHandler) {
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

void intr_set_mode(uint16_t num, gatemode_t mode) {
    if(num >= MAX_INTR)
        fatal("gate %d not available\n", num);

    intr_gate_desc_t* gate = &intr_gate_table[num];

    if(gate->mode & mode)
        return;

    gate->mode = mode;

    if(gate->h.list) {
        list_clear(gate->h.list);

        if(mode != GateModeMultiHandler)
            gate->h.list = list_delete(gate->h.list);
    }

    gate->h.handler = NULL;
}

bool intr_state() {
    register uint64_t rflags = 0;

    asm volatile("pushf; popq %0" : "=r"(rflags));

    return (rflags & FL_IF);
}

void intr_disable() {
    asm volatile("cli");
    thr_ctx_get()->ifda_cnt++;
}

bool intr_enable(bool doIt) {
    register thr_context_t* ctx = thr_ctx_get();

    if(ctx->ifda_cnt > 0)
        ctx->ifda_cnt--;

    if(ctx->ifda_cnt == 0) {
        if(doIt)
            asm volatile("sti");

        return true;
    }

    return false;
}

