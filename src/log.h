/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

typedef enum {
    Fatal = 0,
    Error,
    Warning,
    Info,
    Debug,
    Trace,
} log_level_t;

#define LOG0(lvl, ...) log_write(lvl, __VA_ARGS__, NULL);

#define fatal(...)  { LOG0(Fatal, __VA_ARGS__); abort(); }
#define error(...)  { LOG0(Error, __VA_ARGS__); }
#define warn(...)   { LOG0(Warning, __VA_ARGS__); }
#define info(...)   { LOG0(Info, __VA_ARGS__); }
#define debug(...)  { LOG0(Debug, __VA_ARGS__); }
#define trace(...)  { LOG0(Trace, __VA_ARGS__); }

void log_init();
void log_write(log_level_t lvl, char const* fmt, ...);
void log_set_level(char const* dest, log_level_t lvl);

