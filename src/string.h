/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

int8_t strncmp(char const *p, char const *q, size_t n);
int8_t strcmp(char const *p, char const *q);
char* strncpy(char *s, char const *t, size_t n);
size_t strlen(char const *s);
