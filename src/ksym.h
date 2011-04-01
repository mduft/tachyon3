/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

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

