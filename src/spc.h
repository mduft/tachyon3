/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

/**
 * Creates a new address space, also mapping the kernel
 * space to it, so we can immediately switch to it.
 *
 * @return a new address space.
 */
spc_t spc_new();

/**
 * Deletes an address space, freeing all associated
 * management structures, and all physical memory
 * allocated held by it.
 * 
 * @param target the address space to destroy.
 */
void spc_delete(spc_t target);

/**
 * Retrieves the currently active address space.
 *
 * @return the active address space.
 */
spc_t spc_current();

/**
 * Switches to another address space, making target
 * the current one.
 *
 * @param target the address space to activate.
 */
void spc_switch(spc_t target);

