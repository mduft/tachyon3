/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

/**
 * Maps a specified physical address to a virtual one.
 *
 * @param aspace    the address space to use.
 * @param phys      the physical address to map.
 * @param virt      the virtual address to map to.
 * @param flags     flags controlling the kind of mapping.
 *                  Allowed are (on x86):
 *                      - PG_WRITABLE
 *                      - PG_USER
 *                      - PG_WRITETHROUGH
 *                      - PG_NONCACHABLE
 *                      - PG_LARGE
 *                      - PG_GLOBAL
 *                      - PG_EXECUTE_DISABLE
 * @return          TRUE on success, FALSE otherwise.
 */
bool vmem_map(aspace_t aspace, phys_addr_t phys, uintptr_t virt, uint32_t flags);

/**
 * Assures that a given virtual address is not mapped.
 * Does nothing (except writing a warning), if the given
 * virtual address does not match a mapping exactly.
 *
 * @param aspace    the address space to use.
 * @param virt      the virtual address to unmap.
 */
void vmem_unmap(aspace_t aspace, uintptr_t virt);

/**
 * Tries to find the physical address for a virtual one.
 * If the virtual address is not mapped to a physical one,
 * the function fails.
 *
 * @param  aspace the target address space.
 * @param  virt   the virtual address to resolve.
 * @return        the physical address, or 0 on failure.
 */
bool vmem_resolve(aspace_t aspace, uintptr_t virt, phys_addr_t* target);

