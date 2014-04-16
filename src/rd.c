/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "tachyon.h"
#include "rd.h"
#include "vfs.h"
#include "extp.h"
#include "mboot.h"
#include "path.h"
#include "log.h"
#include "kheap.h"

static fs_desc_t rd_desc = { "initial RAM disk", "rd" };

static fs_ops_t rd_ops = {
    .fs = &rd_desc,
    .open = NULL,
    .close = NULL,
    .read = NULL,
    .write = NULL,
    .seek = NULL,
    .list = NULL,
    .mkdir = NULL
};

static rd_header_t* rd;

static void rd_init() {
    /* find and protect initial ram disc */
    rd = mboot_find_rd();

    if(rd != NULL) {
        debug("rd found at %p, %d files\n", rd, rd->num_files);
        vfs_mount(path_create("/rd", &kheap), rd_ops);
    }
}

INSTALL_EXTENSION(EXTP_VFS_INIT, rd_init, "rd");
