/* Copyright (c) 2023 by Markus Duft <markus.duft@ssi-schaefer.com>
 * This file is part of the 'tachyon' operating system. */

#include "char.h"

bool isspace(char c) {
    switch(c) {
        case ' ':
        case '\n':
        case '\t':
        case '\v':
        case '\f':
        case '\r':
            return true;
    }
    return false;
}

