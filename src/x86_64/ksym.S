# Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
# This file is part of the 'tachyon' operating system.

#include <x86_64/defs.h>

.global ksym_get_bp

ksym_get_bp:
    mov %rbp, %rax
    ret
