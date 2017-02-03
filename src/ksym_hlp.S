# Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
# This file is part of the 'tachyon' operating system.

#include <defs.h>

.global ksym_get_bp

ksym_get_bp:
    mov %rbp, %rax
    ret
