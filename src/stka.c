/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "stka.h"
#include "kheap.h"
#include "pmem.h"
#include "log.h"
#include "vmem.h"
#include "mem.h"

/** size of one chunk of stack space (must be vmem_map'able). */
#define STK_PAGESIZE    0x1000

/** initial size of new stacks */
#define STK_INITSIZE    (STK_PAGESIZE * 2)

/** size of reserved "guard" space into which the stack may grow. */
#define STK_GUARDSIZE   (STK_PAGESIZE * 256)

stack_allocator_t* stka_new(stack_allocator_desc_t* desc) {
    stack_allocator_t* allocator = kheap_alloc(sizeof(stack_allocator_t));

    allocator->desc = *desc;
    allocator->next_stk = allocator->desc.top;
    allocator->stacks = list_new();

    spl_init(&allocator->lock);

    return allocator;
}

stack_allocator_t* stka_delete(stack_allocator_t* allocator) {
    spl_lock(&allocator->lock);

    if(allocator->stacks) {
        list_node_t* node = list_begin(allocator->stacks);

        while(node) {
            stka_free(allocator, (stack_t*)node->data);
            node = node->next;
        }

        list_delete(allocator->stacks);
    }

    kheap_free(allocator);

    spl_unlock(&allocator->lock);
    return NULL;
}

static bool stka_grow(stack_allocator_t* allocator, stack_t* stack, uintptr_t amount) {
    if(amount % STK_PAGESIZE != 0) {
        error("stack growth amount not divisable by page size!\n");
        return false;
    }

    if(stack->mapped - amount <= stack->guard) {
        error("cannot grow stack - maximum size exceeded\n");
        return false;
    }

    for(uintptr_t i = stack->mapped - amount; i < stack->mapped; i += STK_PAGESIZE) {
        phys_addr_t phys = pmem_alloc(STK_PAGESIZE, sizeof(uintptr_t));

        if(!vmem_map(allocator->desc.space, phys,
                (void*)(i), allocator->desc.pg_fl)) {
            error("cannot map page for new stack!\n");
            // TODO: unmap already mapped pages!
            return false;
        }

        memset((void*)(i), 0, STK_PAGESIZE);
    }

    stack->mapped -= amount;
    return true;
}

stack_t* stka_alloc(stack_allocator_t* allocator) {
    spl_lock(&allocator->lock);

    // ATTENTION: since we're allocating stacks: this all works
    //            top to bottom!
    if(allocator->next_stk <= allocator->desc.bottom || 
        (allocator->next_stk - STK_INITSIZE) < allocator->desc.bottom) {
        error("cannot allocate new stack! out of stack space!\n");
        return NULL;
    }

    stack_t* stack = kheap_alloc(sizeof(stack_t));
    memset(stack, 0, sizeof(stack_t));

    stack->top = allocator->next_stk;
    stack->mapped = stack->top;
    stack->guard = stack->top - STK_GUARDSIZE;

    stka_grow(allocator, stack, STK_INITSIZE);

    // leave an additional page room for the next stack, so a stack overflow
    // does not doom another thread, but rather causes a page fault.
    allocator->next_stk = stack->guard + (STK_PAGESIZE);

    list_add(allocator->stacks, stack);

    spl_unlock(&allocator->lock);

    return stack;
}

void stka_free(stack_allocator_t* allocator, stack_t* stack) {
    for(uintptr_t m = stack->mapped; m < stack->top; m += STK_PAGESIZE) {
        phys_addr_t p = vmem_resolve(allocator->desc.space, (void*)m);

        if(p) {
            pmem_free(p, STK_PAGESIZE);
        }

        vmem_unmap(allocator->desc.space, (void*)m);
    }

    list_remove(allocator->stacks, stack);

    kheap_free(stack);
}
