/* Copyright (c) 2023 by Markus Duft <markus.duft@ssi-schaefer.com>
 * This file is part of the 'tachyon' operating system. */

#include "trim.h"
#include "char.h"
#include "string.h"
#include "mem.h"

char* ltrim(char *const s) {
    size_t len;
    char *cur;

    if(s && *s) {
        len = strlen(s);
        cur = s;

        while(*cur && isspace(*cur)) {
            ++cur, --len;
        }

        if(s != cur) {
            memmove(s, cur, len + 1);
        }
    }

    return s;
}

char* rtrim(char *const s) {
    size_t len;
    char *cur;

    if(s && *s) {
        len = strlen(s);
        cur = s + len - 1;

        while(cur != s && isspace(*cur)) {
            --cur, --len;
        }

        cur[isspace(*cur) ? 0 : 1] = '\0';
    }

    return s;
}

char* trim(char *const s) {
    return rtrim(ltrim(s));
}

