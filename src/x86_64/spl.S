# Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
# This file is part of the 'tachyon' operating system.

#include <x86_64/defs.h>

.global spl_lock
.global spl_try_lock
.global spl_unlock
.global spl_locked
.global spl_mine

.type spl_lock, @function
.type spl_try_lock, @function
.type spl_unlock, @function
.type spl_locked, @function
.type spl_mine, @function

.size spl_lock, .spl_lock_end - spl_lock
.size spl_try_lock, .spl_try_lock_end - spl_try_lock
.size spl_unlock, .spl_unlock_end - spl_unlock
.size spl_locked, .spl_locked_end - spl_locked
.size spl_mine, .spl_mine - spl_mine

spl_lock:
    push %rbx
    mov $0x1, %rbx              # target value (TODO: use current cpu's id)

    1:                          # TODO: limit loop count and sleep on every N iterations
    mov $0x0, %rax              # check whether lock is free.
    lock cmpxchg %rbx, (%rdi)   # compare and exchange.
    jz 2f                       # if zero, we've got the lock
    pause                       # hint the cpu to improve performance
    jmp 1b                      # and loop back

    2:
    pop %rbx
    ret
.spl_lock_end:

spl_try_lock:
    push %rbx
    mov $0x0, %rax              # check whether lock is free.
    mov $0x1, %rbx              # target value (TODO: use current cpu's id)
    lock cmpxchg %rbx, (%rdi)   # compare and exchange locked
    mov $0x0, %rax              # reset, old value is not used.
    jnz 1f                      # is not zero, lock was not free.
    mov $0x1, %rax              # zero, so success, return = true.
    1:
    pop %rbx
    ret
.spl_try_lock_end:

spl_unlock:
    movq $0x0, (%rdi)           # a simple mov should suffice on newer cpus.
    ret
.spl_unlock_end:

spl_locked:
    mov $0x0, %rax
    cmp %rax, (%rdi)            # test whether locked.
    je 1f                       # free, return false (already in rax).
    mov $0x1, %rax              # locked, return true.
    1:
    ret
.spl_locked_end:
    
spl_mine:
    jmp spl_locked              # TODO: check lock value for current cpu id.
.spl_mine_end:
