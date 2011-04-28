# Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
# This file is part of the 'tachyon' operating system.

#include <x86_64/defs.h>
#include <x86/asm.h>

.global x86_64_ctx_get
.global x86_64_ctx_set

.type x86_64_ctx_get, @function
.type x86_64_ctx_set, @function

.size x86_64_ctx_get, .x86_64_ctx_get_end - x86_64_ctx_get
.size x86_64_ctx_set, .x86_64_ctx_set_end - x86_64_ctx_set

# TODO: per cpu!
the_one_and_only_context:
    .quad 0x0

x86_64_ctx_get:
    mov the_one_and_only_context, %rax
    ret
.x86_64_ctx_get_end:

x86_64_ctx_set:
    mov %rax, the_one_and_only_context
    ret
.x86_64_ctx_set_end:
