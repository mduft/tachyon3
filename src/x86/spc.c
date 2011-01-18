/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include <spc.h>
#include <pmem.h>
#include "paging.h"

spc_t spc_new() {
    return pmem_alloc(PAGE_SIZE_4K, PAGE_SIZE_4K);
}

void spc_delete(spc_t target) {
    /* TODO */
}

spc_t spc_current() {
    register spc_t sp asm("eax");
    asm volatile("mov %%cr3, %0" : "=a"(sp));
    return sp;
}

void spc_switch(spc_t target) {
    asm volatile("mov %0, %%cr3" :: "r"(target));
}
