/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "vfs.h"
#include "extp.h"
#include "log.h"

static void vfs_init_extp(char const* tag, extp_func_t cb, char const* desc) {
    if(cb) {
        debug("VFS: init %s\n", desc);
        cb();
    }
}

void vfs_init() {
    extp_iterate(EXTP_VFS_INIT, vfs_init_extp);
}


bool vfs_mount(path_t* path, fs_ops_t ops) {
    return false;
}

bool vfs_unmount(path_t* path) {
    return false;
}

path_t** vfs_list(path_t* path) {
    return NULL;
}

vfs_handle_t* vfs_open(path_t* path) {
    return NULL;
}

void vfs_close(vfs_handle_t* handle) {

}

ssize_t vfs_read(vfs_handle_t* handle, void* buffer, size_t len) {
    return ERR_UNKNOWN;
}

ssize_t vfs_write(vfs_handle_t* handle, void* buffer, size_t len) {
    return ERR_UNKNOWN;
}

off_t vfs_seek(vfs_handle_t* handle, off_t offset, fs_whence_t whence) {
    return ERR_UNKNOWN;
}

bool vfs_mkdir(path_t* path) {
    return false;
}

