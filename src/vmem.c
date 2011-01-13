/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "vmem.h"
#include "vmem_ps.h"

bool vmem_map(aspace_t aspace, phys_addr_t phys, uintptr_t virt, uint32_t flags) {
    return false; /* TODO */
}

void vmem_unmap(aspace_t aspace, uintptr_t virt) {

}

bool vmem_resolve(aspace_t aspace, uintptr_t virt, phys_addr_t* target) {
    return false; /* TODO */
}

