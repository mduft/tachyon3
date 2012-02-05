/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"
#include "heap.h"
#include "list.h"
#include "spl.h"
#include "spc.h"

/** size of one chunk of stack space (must be vmem_map'able). */
#define STK_PAGESIZE    0x1000

/**
 * Describes the basic information required to create a stack allocator.
 */
typedef struct {
    uintptr_t bottom;    /**< the start of the stack region */
    uintptr_t top;       /**< the end of the stack region */
    spc_t space;         /**< adress space for the stack */
    uintptr_t pg_fl;     /**< paging flag hint */
    bool fixed;          /**< is the stack fixed in size? */
} stack_allocator_desc_t;

/**
 * Describes a single stack.
 */
typedef struct {
    uintptr_t top;      /**< the top of the stack. */
    uintptr_t mapped;   /**< address up until which the stack is currently mapped */
    uintptr_t guard;    /**< address up until which the stacks guard pages are reserved */
} stack_t;

/**
 * Describes a stack allocator.
 */
typedef struct {
    stack_allocator_desc_t desc;    /**< the descriptor used to initalize the allocator. */

    uintptr_t next_stk;             /**< the last allocated stack hint. */
    list_t* stacks;                 /**< the list of all allocated stacks */

    spinlock_t lock;                /**< a lock to prevent damage of internal structures */
} stack_allocator_t;

/**
 * Creates a new stack allocator from the given description.
 *
 * @param desc  the description. all fields have to be valid.
 * @return      a new stack allocator, or NULL on failure.
 */
stack_allocator_t* stka_new(stack_allocator_desc_t* desc);

/**
 * Deletes a stack allocator, freeing all associated management
 * structures from the parent heap.
 *
 * @param allocator the allocator to delete.
 * @return          always NULL.
 */
stack_allocator_t* stka_delete(stack_allocator_t* allocator);

/**
 * Allocates a new stack and returns the descriptor.
 *
 * @param allocator the allocator to allocate from.
 * @return          the top address of the new stack.
 */
stack_t* stka_alloc(stack_allocator_t* allocator);

/**
 * Frees a given stack.
 *
 * @param allocator     the allocator to return the stack to.
 * @param stack         the stack descriptor
 */
void stka_free(stack_allocator_t* allocator, stack_t* stack);

/**
 * Tries to handle a pagefault inside the given stack's region.
 *
 * @param allocator     the stack's parent allocator.
 * @param stack         the stack in which the fault occured.
 * @param fault         the faulting address within the stack space.
 * @return              whether the page fault was handled.
 */
bool stka_pgflt(stack_allocator_t* allocator, stack_t* stack, uintptr_t fault);
