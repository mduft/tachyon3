/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

/* 
 * .----------------------------------.
 * | section based extension points   |
 * '----------------------------------' 
 */

typedef void (*extp_func_t)(void);
typedef void (*extp_iterator_func_t)(char const*, extp_func_t);

typedef struct {
    char const* const ext_tag;
    extp_func_t ext_func;
} extension_point_t;

#define SECTION(x)  __attribute__((section(x)))
#define UNUSED      __attribute__((unused))

#define INSTALL_EXTENSION(t, e) \
    static extension_point_t __extp_##t##e UNUSED SECTION(".extp") = { #t, (extp_func_t)e } ;

void extp_iterate(char const* tag, extp_iterator_func_t callback);

