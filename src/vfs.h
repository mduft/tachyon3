/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "fs.h"

/**
 * Describes a single VFS object, which wraps around a filesystem
 * relative object that is returned by the relevant fs functions.
 */
typedef struct {
    fs_obj_t fso;       /**< the filesystem relative object */
    path_t* vpath;      /**< the VFS (complete) path to the object */
    fs_ops_t* ops;      /**< filesystem operations applicable for this and all children. */
} vfs_obj_t;

/**
 * Describes a handle to a VFS object that has been opened with vfs_open
 */
typedef struct {
    vfs_obj_t* obj;     /**< the original vfs object */
    fs_handle_t handle; /**< the underlying filesystem's handle */
} vfs_handle_t;

/**
 * initializes the virtual file system
 */
void vfs_init();

bool vfs_mount(path_t* path, fs_ops_t ops);

void vfs_unmount(path_t* path);

path_t** vfs_list(path_t* path);

vfs_handle_t* vfs_open(path_t* path);

void vfs_close(vfs_handle_t* handle);
