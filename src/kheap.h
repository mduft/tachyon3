/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"
#include "heap.h"

/** helper macros around heap_* for the kernel heap */
#define kheap_alloc(x)      heap_alloc(&kheap, x)
#define kheap_free(x)       heap_free(&kheap, x)
#define kheap_realloc(x, s) heap_realloc(&kheap, x, s)
#define kheap_state()       kheap.state

/**
 * The actual kernel heap.
 */
extern heap_t kheap;

/**
 * Initializes the kernels internal heap.
 */
void kheap_init();

