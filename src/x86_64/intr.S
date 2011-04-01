# Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
# This file is part of the 'tachyon' operating system.

#include <x86_64/defs.h>
#include <x86/cpu.h>

.section .text

.global intr_state
.global intr_enable
.global intr_disable

.type intr_state, @function
.type intr_enable, @function
.type intr_disable, @function

.size intr_state, .intr_state_end - intr_state
.size intr_enable, .intr_enable_end - intr_enable
.size intr_disable, .intr_disable_end - intr_disable

# .----------------------------------------.
# | returns the state of interrupt delivery|
# | on the current CPU in rax              |
# '----------------------------------------'

intr_state:
    pushf
    pop %rax
    and $FL_IF, %rax

    # convert != 0 to a 1 (true)
    jz 1f
    mov $1, %eax
    1:
    ret
.intr_state_end:

# .----------------------------------------.
# | enables interrupt if a per-cpu count   |
# | reaches zero.                          |
# '----------------------------------------'

intr_enable:
    # TODO: per-cpu count.
    sti
    ret
.intr_enable_end:

# .----------------------------------------.
# | disables interrupts if a per-cpu count |
# | is zero. otherwise increase count.     |
# '----------------------------------------'

intr_disable:
    # TODO: per-cpu count.
    cli
    ret
.intr_disable_end:
