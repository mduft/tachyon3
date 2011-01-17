# Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
# This file is part of the 'tachyon' operating system.

#include <x86_64/defs.h>

.global spl_lock
.global spl_try_lock
.global spl_unlock

spl_lock:
    # TODO
    ret

spl_try_lock:
    # TODO
    mov $0, %rax
    ret

spl_unlock:
    # TODO
    ret
