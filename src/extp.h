/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

/* 
 * .----------------------------------.
 * | predefined extension points      |
 * '----------------------------------' 
 */

#define EXTP_LOG_WRITER     "log.writer"

typedef void (*extp_func_t)(void);
typedef void (*extp_iterator_func_t)(char const*, extp_func_t, char const*);

typedef struct {
    char const* const ext_tag;
    extp_func_t ext_func;
    char const* const ext_descr;
} extension_point_t;

#define INSTALL_EXTENSION(t, e, d) \
    static extension_point_t __extp_##e UNUSED SECTION(".extp") = { t, (extp_func_t)e, d } ;

void extp_iterate(char const* tag, extp_iterator_func_t callback);

