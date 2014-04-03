/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "path.h"

/**
 * Describes the type of a VFS object
 */
typedef enum {
    File,
    Directory,
    Link
} vfs_type_t;

/**
 * Describes a single VFS object
 */
typedef struct {
    vfs_type_t type;
    path_t path;
} vfs_object_t;

typedef vfs_object_t* (*vfs_open_t)(path_t* path);      /**< opens a VFS object */
typedef vfs_object_t** (*vfs_list_t)(vfs_object_t*);    /**< lists VFS objects children (must be Directory) */
typedef void (*vfs_close_t)(vfs_object_t*);             /**< closes previously openend VFS handle */

/**
 * Defines the VFS operations to use for a certain path.
 */
typedef struct {
    vfs_open_t open;
    vfs_list_t list;
    vfs_close_t close;
} vfs_ops_t;
