/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

void* memset(void *dst, uint8_t c, size_t n);
int8_t memcmp(void const *v1, void const *v2, size_t n);
void* memcpy(void *dst, void const *src, size_t n);
void* memmove(void *dst, void const *src, size_t n);

