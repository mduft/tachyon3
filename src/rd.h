/* Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
 * This file is part of the 'tachyon' operating system. */

#pragma once

/*
 * ATTENTION: since this file is included both from the host disc tool
 *   as well as the kernel itself, this file is not allowed to include
 *   anything else. The user has to include the appropriate file before.
 */

#define RD_MAGIC 0x44524452

/**
 * Describes the init ram disk file header
 */
typedef struct {
    uint32_t magic;     /**< magic number to verify format */
    uint32_t hdr_size;  /**< this headers size */
    uint32_t num_files; /**< number of following file headers */
} PACKED rd_header_t;

/**
 * Describes a single file that is contained in the image
 */
typedef struct {
    uint32_t hdr_size;  /**< this headers size */
    uint32_t start;     /**< start offset from beginning of file */
    uint32_t size;      /**< size of the file in bytes */
    char name[64];      /**< name of the file, including trailing zero */
} PACKED rd_file_t;

