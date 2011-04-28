# Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
# This file is part of the 'tachyon' operating system.

#include <x86_64/defs.h>
#include <x86/asm.h>

.global x86_64_ctx_get
.type x86_64_ctx_get, @function
.size x86_64_ctx_get, .x86_64_ctx_get_end - x86_64_ctx_get

x86_64_ctx_get:
    mov $the_one_and_only_context, %rax
    ret
.x86_64_ctx_get_end:

