/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include <aspace.h>

aspace_t aspace_new() {
    /* TODO */
    return 0;
}

void aspace_delete(aspace_t target) {
    /* TODO */
}

aspace_t aspace_current() {
    register aspace_t sp asm("eax");
    asm volatile("mov %%cr3, %0" : "=a"(sp));
    return sp;
}

void aspace_switch(aspace_t target) {
    asm volatile("mov %0, %%cr3" :: "r"(target));
}
