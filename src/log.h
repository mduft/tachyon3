/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

/**
 * The different levels a log an be written to.
 */
typedef enum {
    Fatal = 0,  /**< fatal error. should be written for unrecoverable errors. */
    Error,      /**< error. should be written in case of recoverable errors. */
    Warning,    /**< warning. in case an error could happen (fex suspicous data). */
    Info,       /**< info. something of interest to the user, but not a problem. */
    Debug,      /**< debug. only used for kernel development. */
    Trace,      /**< trace. also: only kernel development, even more fine grained. */
} log_level_t;

/**
 * Just a small wrapper macro, which adds a terminating NULL on the
 * way to log_write(), as otherwise calling the following macros
 * with just a format string, and without parameters, would result
 * in a compile error.
 */
#define LOG0(lvl, str, ...) log_write(lvl, str __VA_ARGS__, NULL);

#define fatal(...)  { LOG0(Fatal,   "fatal: ", __VA_ARGS__); abort(); }
#define error(...)  { LOG0(Error,   "error: ", __VA_ARGS__); }
#define warn(...)   { LOG0(Warning, "warn:  ", __VA_ARGS__); }
#define info(...)   { LOG0(Info,    "info:  ", __VA_ARGS__); }
#define debug(...)  { LOG0(Debug,   "debug: ", __VA_ARGS__); }
#define trace(...)  { LOG0(Trace,   "trace: ", __VA_ARGS__); }

/**
 * Initializes the logging subsystem. This gathers log writers from
 * the EXTP_LOG_WRITER extension point. This means every device that
 * is capable of writing the log (the screen, serial interface, etc.),
 * can define such an extension point, and will be added here with
 * the default logging level.
 */
void log_init();

/**
 * Formats and writes a string to the log. The message is multicast
 * to all registered log writers, of the associated level matches.
 *
 * @param lvl   the log level of the message.
 * @param fmt   the format string (printf like).
 * @param ...   the format parameters for the given format string.
 */
void log_write(log_level_t lvl, char const* fmt, ...);

/**
 * Sets the acceptable log level for a specific log writer (destination),
 * or the accepted level for all writers (if dest == NULL).
 *
 * @param dest  the destination to change (NULL for all destinations).
 * @param lvl   the target log level to accept from now on.
 */
void log_set_level(char const* dest, log_level_t lvl);

