/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include <string.h>
#include <stdio.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define PACKED __attribute__((__packed__))
#include "../../../src/rd.h"

#define LOG(str, ...) if(verbose) { printf(str, __VA_ARGS__); }

#define ERR_ARGUMENT 1
#define ERR_FILENAME 2
#define ERR_OSCALL   3
#define ERR_NOIMAGE  4
#define ERR_HDRSIZE  5
#define ERR_ISDIR    6

#define CHECK_CALL(x, func) if((int)x == -1) {           \
                                perror(func);       \
                                return ERR_OSCALL;  \
                            }

int create(char* name, int nfiles, char** filenames);
int extract(char* name, int test);

int verbose = 0;

int main(int argc, char ** argv) {
    if(argc < 3) {
        printf("usage: %s [tcxv] <image> [file ...]\n", argv[0]);
        return ERR_ARGUMENT;
    }

    int mode_test       = strchr(argv[1], 't') == NULL ? 0 : 1;
    int mode_create     = strchr(argv[1], 'c') == NULL ? 0 : 1;
    int mode_extract    = strchr(argv[1], 'x') == NULL ? 0 : 1;
    verbose             = strchr(argv[1], 'v') == NULL ? 0 : 1;

    if((mode_test + mode_create + mode_extract) > 1) {
        printf("only one of 't', 'c' or 'x' may be given\n");
        return ERR_ARGUMENT;
    }

    if(mode_create) {
        if(argc < 4) {
            printf("no files to add to the image given\n");
            return ERR_ARGUMENT;
        }
        return create(argv[2], argc - 3, &argv[3]);
    } else {
        return extract(argv[2], mode_test);
    }
}

int create(char* name, int nfiles, char** filenames) {
    int i, pos = 0;
    rd_header_t hdr = { RD_MAGIC, sizeof(rd_header_t), nfiles };

    LOG("creating image (nfiles=%d)\n", nfiles);

    int target = open(name, O_CREAT | O_WRONLY, 0664);
    CHECK_CALL(target, "open target file");

    ssize_t bw = write(target, &hdr, sizeof(hdr));
    CHECK_CALL(bw, "write header");
    pos += sizeof(hdr);
    
    for(i = 0; i < nfiles; ++i) {
        struct stat sbuf;
        rd_file_t fhdr;

        if(strlen(filenames[i]) > (sizeof(fhdr.name) - 1)) {
            printf("filename too long: %s\n", filenames[i]);
            return ERR_FILENAME;
        }

        // retrieve file size of source.
        int res = stat(filenames[i], &sbuf);
        CHECK_CALL(res, "retrieve file size");

        if(S_ISDIR(sbuf.st_mode)) {
            printf("error: directories not supported: %s\n", filenames[i]);
            return ERR_ISDIR;
        }

        // poor mans basename
        char * basename = strrchr(filenames[i], '/') + 1;

        // fill out all file header fields.
        fhdr.hdr_size = sizeof(rd_file_t);
        fhdr.start = pos + sizeof(fhdr);
        fhdr.size = sbuf.st_size;
        strncpy(fhdr.name, basename, sizeof(fhdr.name));

        LOG("adding: %s (%d bytes @ 0x%x)\n", basename, fhdr.size, fhdr.start);

        // write file header to target
        bw = write(target, &fhdr, sizeof(fhdr));
        CHECK_CALL(bw, "write file header");

        // now open source file and copy over to other fd.
        int source = open(filenames[i], O_RDONLY);
        CHECK_CALL(source, "open source file");

        bw = sendfile(target, source, NULL, fhdr.size);
        CHECK_CALL(bw, "copy source file to target");
        close(source);

        pos = pos + sizeof(fhdr) + fhdr.size;
    }
    close(target);
}

#define TEST(str, ...) if(test) { printf(str, __VA_ARGS__); } else { LOG(str, __VA_ARGS__); }
int extract(char* name, int test) {
    int source = open(name, O_RDONLY);
    CHECK_CALL(source, "open source");

    rd_header_t hdr;
    ssize_t br = read(source, &hdr, sizeof(hdr));
    CHECK_CALL(br, "read header");

    if(hdr.magic != RD_MAGIC) {
        printf("not a valid image file, magic mismatch (0x%x != 0x%x)\n", hdr.magic, RD_MAGIC);
        return ERR_NOIMAGE;
    }

    if(hdr.hdr_size != sizeof(hdr)) {
        printf("header size differs, image created by a different version of this tool or corrupted\n");
        return ERR_HDRSIZE;
    }

    LOG("reading image %s (nfiles=%d)\n", name, hdr.num_files);

    while(hdr.num_files-- > 0) {
        rd_file_t fhdr;
        br = read(source, &fhdr, sizeof(fhdr));
        CHECK_CALL(br, "read file header");

        if(fhdr.hdr_size != sizeof(fhdr)) {
            printf("file header size differs, image created by a different version of this tool or corrupted\n");
            return ERR_HDRSIZE;
        }

        TEST("%s (%d @ 0x%x)\n", fhdr.name, fhdr.size, fhdr.start);

        if(!test) {
            int target = open(fhdr.name, O_CREAT | O_EXCL | O_WRONLY, 0664);
            CHECK_CALL(target, "open target file");

            off_t offset = fhdr.start;
            br = sendfile(target, source, &offset, fhdr.size);
            CHECK_CALL(br, "extract file");

            close(target);
        }

        off_t off = lseek(source, fhdr.start + fhdr.size, SEEK_SET);
        CHECK_CALL(off, "seek to next file");
    }

    close(source);
}

