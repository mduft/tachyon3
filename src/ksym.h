/* Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"
#include "log.h"
#include "list.h"

/**
 * Describes a kernel symbol.
 */
typedef struct {
    uintptr_t addr;     /**< the address of the symbol */
    size_t size;        /**< the size in bytes of the symbol */
    char const* name;   /**< the name of the found symbol */
} ksym_t;

typedef struct {
    ksym_t const* sym;
    void* real_addr;
} ksym_node_t;

/**
 * Look up the given symbols name. This searches for the 
 * nearest upper bound symbol that is known in the sorted 
 * kernel symbol table.
 *
 * @param addr  the address within the symbol to find.
 * @return      the name of the symbol
 */
ksym_t const* ksym_get(void* addr);

/**
 * Print a given stack trace.
 *
 * @param level the level to write the trace at.
 * @param trace the trace to write.
 */
void ksym_write_trace(log_level_t level, list_t* trace);

/**
 * Print a part of the given stack trace, limiting the
 * count of items.
 *
 * @param level the level to write the trace at.
 * @param trace the trace to write.
 * @param limit the maximum count of rows.
 */
void ksym_write_trace_top(log_level_t level, list_t* trace, int16_t limit);

/**
 * Retrieve a stack trace from the current stack position.
 * The caller is responsible for calling list_delete() to
 * free resources allocated by the trace. The ksym_t* ers
 * need not be freed.
 *
 * @return a list containing ksym_t*
 */
list_t* ksym_trace();

/**
 * Destroy a previously retrieved trace.
 *
 * @param trace the trace, retrieved via ksym_trace()
 */
void ksym_delete(list_t* trace);

/**
 * Helper to get the current base pointer of the calling function.
 *
 * @return the base pointer
 */
uintptr_t* ksym_get_bp();
