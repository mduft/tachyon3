/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
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
 * Print a current stack trace.
 *
 * @param the level to write the trace at.
 */
void ksym_write_trace(log_level_t level, list_t* trace);

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
 * Helper to get the current base pointer of the calling function.
 *
 * @return the base pointer
 */
uintptr_t* ksym_get_bp();
