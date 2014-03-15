/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include <stdio.h>
#include <inttypes.h>

#include <rd.h>

int main(int argc, char ** argv) {
    int i;

    if(argc < 3) {
        printf("usage: %s [tcxv] <image> [file ...]"
    }

    for(i = 0; i < argc; ++i) {
        printf("%s\n", argv[i]);
    }
}
