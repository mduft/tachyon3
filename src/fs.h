/* Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "path.h"

#define NO_HANDLE ((uintptr_t)-1)

#define ERR_UNKNOWN     -1
#define ERR_EOF         -2
#define ERR_READONLY    -3

/**
 * Defines the mode that a filesystem object is to be opened with
 */
typedef enum {
    ReadOnly, WriteOnly, ReadWrite
} fs_mode_t;

/**
 * Defines the offset to use when repositioning within a file
 */
typedef enum {
    Start, Current, End
} fs_whence_t;

/**
 * Describes the type of a FS object
 */
typedef enum {
    File,
    Directory,
    Link
} fs_objtype_t;

typedef struct {
    char* id;
    char* type;
} fs_desc_t;

/**
 * Describes a filesystem object on the filesystem
 */
typedef struct {
    fs_objtype_t type; /**< the type of the current object. */
    path_t* path;      /**< the path within the mounted filesystem to this object. */
} fs_obj_t;

typedef uintptr_t fs_handle_t;                                  /**< denotes a file handle */
typedef fs_obj_t (*fs_get)(path_t*);                            /**< retrieve information about the path */
typedef fs_handle_t (*fs_open)(path_t*, fs_mode_t);             /**< opens a file from the fs */
typedef void (*fs_close)(fs_handle_t);                          /**< closes a file from the fs */
typedef ssize_t (*fs_read)(fs_handle_t, void*, size_t);         /**< reads contents of a file */
typedef ssize_t (*fs_write)(fs_handle_t, void*, size_t);        /**< writes contents to a file */
typedef off_t (*fs_seek)(fs_handle_t, off_t, fs_whence_t);      /**< repositions the cursor in a file */
typedef path_t** (*fs_list)(fs_handle_t);                       /**< lists all available children */
typedef bool (*fs_mkdir)(path_t*);                              /**< creates the given path in the filesystem */

/**
 * Defines all operations that can be applied to FS objects
 */
typedef struct __fs_ops_t {
    fs_desc_t* fs;
    fs_open open;
    fs_close close;
    fs_read read;
    fs_write write;
    fs_seek seek;
    fs_list list;
    fs_mkdir mkdir;
} fs_ops_t;

