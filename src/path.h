/* Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "heap.h"

#define PATH_SEPERATOR "/"
#define PATH_CURDIR "."
#define PATH_PARENT ".."

/**
 * Describes a path in the VFS. The path is broken into components.
 * <p>
 * Note: path_t is <b>always</b> absolute.
 */
typedef struct {
    heap_t* heap;       /**< the heap to use for memory allocations */
    size_t count;     /**< the count of components of the path */
    char ** components; /**< the individual components of the path */
} path_t;

/**
 * Splits the given string by the path seperator, and returns a path_t
 * representation of the path.
 *
 * @param path an <b>absolute</b> path to split.
 * @param heap the heap used for memory allocation, also in all
 *             subsequent operations related to this instance.
 * @return a path_t instance representing this path or NULL on error.
 */
path_t* path_create(char const* path, heap_t* heap);

/**
 * Joins the given path_t's components to form a string representation
 * of the denoted path.
 *
 * @param path the path to create a string representation for.
 * @return a string representing the given path.
 */
char* path_string(path_t* path);
