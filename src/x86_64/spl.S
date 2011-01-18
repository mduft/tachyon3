# Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
# This file is part of the 'tachyon' operating system.

#include <x86_64/defs.h>

.global spl_lock
.global spl_try_lock
.global spl_unlock
.global spl_locked
.global spl_mine

spl_lock:
    push %rbx
    push %rcx
    mov $0x0, %rax
    mov $0x1, %rbx                      # TODO: use current CPU id.
    1:
        mov $0x3, %rcx                  # only try to actively spin some times.
        2:
            lock cmpxchg %rbx, (%rdi)
            jz 3f

            pause
            loop 2b

    # TODO: give up thread's timeslice, and re-schedule.
    jmp 1b

    3:
    pop %rcx
    pop %rbx
    ret

spl_try_lock:
    push %rbx
    mov $0x0, %rax
    mov $0x1, %rbx              # TODO: use current CPU id. 
    lock cmpxchg %rbx, (%rdi)
    jnz 1f                      # if ZF, success, else ret false (RAX = 0)
    mov $0x1, %rax              # contains non-zero here. alias to "true"

    1:
    pop %rbx
    ret

spl_unlock:
    movq $0x0, (%rdi)            # should do on any newer CPU. no locking needed.
    ret

spl_locked:
    cmp $0x0, (%rdi)
    jz 1f

    mov $0x1, %rax

    1:
    ret
    
spl_mine:
    jmp spl_locked              # TODO: compare locks value with current CPU id
