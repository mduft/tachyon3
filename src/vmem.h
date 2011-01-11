/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

void vmem_init();

void vmem_map(aspace_t aspace, phys_addr_t phys, uintptr_t virt, uint32_t flags);

void vmem_unmap(aspace_t aspace, uintptr_t virt);

phys_addr_t vmem_resolve(aspace_t aspace, uintptr_t virt);

