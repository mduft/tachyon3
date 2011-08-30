/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

/* 
 * .----------------------------------.
 * | predefined extension points      |
 * '----------------------------------' 
 */

#define EXTP_PMEM_REGION    "pmem.region"
#define EXTP_TIMERGEN       "timer.gen"
#define EXTP_SYSTIME        "sys.time"

/**
 * Dummy extension point callback type. The real type depends
 * on the extension point itself, and is casted to the correct
 * type, when the extension points are used.
 */
typedef void (*extp_func_t)(void);

/**
 * Callback signature for an iterator function, used in conjunction
 * with the extp_iterate function.
 *
 * @param tag       the extp identifier.
 * @param cb        the callback for this extension.
 * @param descr     the description string for this extension.
 */
typedef void (*extp_iterator_func_t)(char const* tag, extp_func_t cb, char const* descr);

/**
 * Structure of an extension point, both in memory, and linked
 * into the executable (the '.extp' section contains an array
 * of such descriptors).
 */
typedef struct {
    char const* const ext_tag;      /**< the identifier of the extension point */
    extp_func_t ext_func;           /**< the callback function to register. */
    char const* const ext_descr;    /**< an optional string, interpreted by the extension */
} PACKED extension_point_t;

/**
 * Installs a specified extension point to the .extp section
 * in the kernel.
 */
#define INSTALL_EXTENSION(t, e, d) \
    static extension_point_t __extp_##e = { t, (extp_func_t)e, d } ; \
    static extension_point_t* __extpp_##e UNUSED SECTION(".extp") = &__extp_##e;

/**
 * Iterate over all extension points. The callback is called
 * for each extension point that matches the tag criteria. If
 * the given tag is NULL, all extension points are iterated.
 *
 * @param tag       the extension point identifier to accept.
 * @param callback  the callback which is called for each matching extp.
 */
void extp_iterate(char const* tag, extp_iterator_func_t callback);

