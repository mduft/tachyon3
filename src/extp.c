/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "extp.h"
#include "string.h"
#include "ldsym.h"

void extp_iterate(char const* tag, extp_iterator_func_t callback) {
    extension_point_t** cur = &_core_vma_sextp;

    while(cur != &_core_vma_eextp && *cur) {
        if(!tag || strcmp((*cur)->ext_tag, tag) == 0)
            callback((*cur)->ext_tag, (*cur)->ext_func, (*cur)->ext_descr);

        ++cur;
    }
}

