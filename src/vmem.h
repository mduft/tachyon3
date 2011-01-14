/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

/**
 * Calculate the page on which a given address lies.
 *
 * @param a the address to inspect.
 * @param s the page size to assume.
 */
#define VM_PAGE(a, s)   ((a) & ~((s) - 1))

/**
 * Calculate the offset of a given address within it's page.
 *
 * @param a the address to inspect.
 * @param s the page size to assume.
 */
#define VM_OFFSET(a, s) ((a) & ((s) - 1))

/**
 * Used to mask off all paging attributes from a paging structure
 * entry, to get a physical address.
 */
#define VM_ENTRY_FLAG_MASK  (~0xFFF)

/**
 * Maps a specified physical address to a virtual one.
 *
 * @param spc    the address space to use.
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
 * @return          true on success, false otherwise.
 */
bool vmem_map(spc_t spc, phys_addr_t phys, void* virt, uint32_t flags);

/**
 * Assures that a given virtual address is not mapped.
 * Does nothing (except writing a warning), if the given
 * virtual address does not match a mapping exactly.
 *
 * @param spc    the address space to use.
 * @param virt      the virtual address to unmap.
 * @return          the physical address the page was mapped to.
 */
phys_addr_t vmem_unmap(spc_t spc, void* virt);

/**
 * Tries to find the physical address for a virtual one.
 * If the virtual address is not mapped to a physical one,
 * the function fails.
 *
 * @param  spc the target address space.
 * @param  virt   the virtual address to resolve.
 * @return        the physical address, or 0 on failure.
 */
phys_addr_t vmem_resolve(spc_t spc, void* virt);

