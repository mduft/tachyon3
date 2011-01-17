# Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
# This file is part of the 'tachyon' operating system.

#include <x86/defs.h>
#include <x86/asm.h>
#include <x86/cpu.h>

.section .text

.global intr_state
.global intr_enable
.global intr_disable

# .----------------------------------------.
# | returns the state of interrupt delivery|
# | on the current CPU in eax              |
# '----------------------------------------'

intr_state:
    pushf
    pop %eax
    and $FL_IF, %eax

    # convert != 0 to a 1 (true)
    jz 1f
    mov $1, %eax
    1:
    ret

# .----------------------------------------.
# | enables interrupt if a per-cpu count   |
# | reaches zero.                          |
# '----------------------------------------'

intr_enable:
    # TODO: per-cpu count.
    sti
    ret

# .----------------------------------------.
# | disables interrupts if a per-cpu count |
# | is zero. otherwise increase count.     |
# '----------------------------------------'

intr_disable:
    # TODO: per-cpu count.
    cli
    ret
