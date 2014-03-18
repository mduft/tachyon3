/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "../../../src/rd.h"

int main(int argc, char ** argv) {
    int i;
    int mode_test = 0;
    int mode_create = 0;
    int mode_extract = 0;
    int mode_verbose = 0;

    if(argc < 3) {
        printf("usage: %s [tcxv] <image> [file ...]\n", argv[0]);
        return 1;
    }

    mode_test       = strchr(argv[1], 't') == NULL ? 0 : 1;
    mode_create     = strchr(argv[1], 'c') == NULL ? 0 : 1;
    mode_extract    = strchr(argv[1], 'x') == NULL ? 0 : 1;
    mode_verbose    = strchr(argv[1], 'v') == NULL ? 0 : 1;

    if((mode_test + mode_create + mode_extract) > 1) {
        printf("only one of 't', 'c' or 'x' may be given\n");
        return 1;
    }
}
