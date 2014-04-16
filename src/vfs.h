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

/**
 * Creates "virtual" nodes in the VFS, that will be represented as empty
 * directories. These nodes server the purpose to be able to mount
 * arbitrary filesystems to locations that do not normally exist in any
 * of the physical filesystems.
 *
 * @param path the path to create. the final part of the path must not
 *             exist yet.
 * @return true on success, false otherwise.
 */
bool vfs_mkvirtual(path_t* path);

/**
 * Mounts the given filesystem to the given path. The given path node must
 * exist already
 *
 * @param path the path where to mount the filesystem
 * @param ops the filesystem description, contains all operations and info.
 * @return true on success, false otherwise.
 */
bool vfs_mount(path_t* path, fs_ops_t ops);

/**
 * Unmounts the filesystem mounted at the given path, if there is one.
 *
 * @param path path to the filesystem mount to unmount
 * @return true on success, false otherwise. If there was no filesystem
 *         present, true is returned.
 */
bool vfs_unmount(path_t* path);

/**
 * Lists available children of the given path
 *
 * @param path the path to query
 * @return an array of paths that are children of the given path.
 */
path_t** vfs_list(path_t* path);

/**
 * Opens a file from the underlying filesystem at the given path
 *
 * @param path the path of the file to open
 * @return the handle to the file or NULL on error.
 */
vfs_handle_t* vfs_open(path_t* path);

/**
 * Closes a previously opened file
 *
 * @param handle the handle to close.
 */
void vfs_close(vfs_handle_t* handle);

/**
 * Reads from a previously opened file.
 *
 * @param handle the handle to the file to read from.
 * @param buffer a buffer of size len (at least) that will be filled.
 * @param len the maximum length to read from the file.
 * @return the number of bytes read from the file, or an error code.
 */
ssize_t vfs_read(vfs_handle_t* handle, void* buffer, size_t len);

/**
 * Writes to a previously opened file.
 *
 * @param handle the handle to the file to write to.
 * @param buffer a buffer of size len (at least) that will be read from.
 * @param len the amount of bytes to read from buffer and write to the file.
 * @return the number of bytes written to the file, or an error code.
 */
ssize_t vfs_write(vfs_handle_t* handle, void* buffer, size_t len);

/**
 * Position the cursor in a file to a certain offset. Subsequent read/write
 * operations will read/write from/to this position.
 *
 * @param handle the file handle to modify
 * @param offset the offset from whence to position to.
 * @param whence base for calculating the offset.
 * @return the new position in the file, or an error code.
 */
off_t vfs_seek(vfs_handle_t* handle, off_t offset, fs_whence_t whence);

/**
 * Creates the given directory in the underlying filesystem for the given
 * path.
 *
 * @param path the path to create
 * @return true on success, false otherwise
 */
bool vfs_mkdir(path_t*);

