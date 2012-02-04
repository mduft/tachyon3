# Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
# This file is part of the 'tachyon' operating system.

#include <defs.h>

.global spl_init
.global spl_lock
.global spl_try_lock
.global spl_unlock
.global spl_locked
.global spl_mine

.type spl_init, @function
.type spl_lock, @function
.type spl_try_lock, @function
.type spl_unlock, @function
.type spl_locked, @function
.type spl_mine, @function

.size spl_init, .spl_init_end - spl_init
.size spl_lock, .spl_lock_end - spl_lock
.size spl_try_lock, .spl_try_lock_end - spl_try_lock
.size spl_unlock, .spl_unlock_end - spl_unlock
.size spl_locked, .spl_locked_end - spl_locked
.size spl_mine, .spl_mine_end - spl_mine

spl_init:
    movq $0x0, (%rdi)
    ret
.spl_init_end:

spl_lock:
    push %rbx
    push %rdi
    call intr_disable
    call cpu_current_id
    pop %rdi
    add $1, %rax                # argl... cpu with id 0...?
    mov %rax, %rbx              # target value (current cpu's id)

    1:                          # TODO: limit loop count and sleep on every N iterations
    mov $0x0, %rax              # check whether lock is free.
    lock cmpxchg %rbx, (%rdi)   # compare and exchange.
    jz 2f                       # if zero, we've got the lock
    pause                       # hint the cpu to improve performance
    jmp 1b                      # and loop back

    2:
    mov $0x1, %rdi
    call intr_enable
    pop %rbx
    ret
.spl_lock_end:

spl_try_lock:
    push %rbx
    push %rdi
    call intr_disable
    call cpu_current_id
    pop %rdi
    add $1, %rax                # see above.
    mov %rax, %rbx              # target value (current cpu's id)
    mov $0x0, %rax              # check whether lock is free.
    lock cmpxchg %rbx, (%rdi)   # compare and exchange locked
    mov $0x0, %rax              # reset, old value is not used.
    jnz 1f                      # is not zero, lock was not free.
    mov $0x1, %rax              # zero, so success, return = true.
    1:
    mov $0x1, %rdi
    call intr_enable
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
    call cpu_current_id
    add $1, %rax                # see above.
    cmp %rax, (%rdi)            # check whether our ID is in there...
    je 1f                       # we have it! return true;
    mov $0x0, %rax              # it's somebody else's
    ret
    1:
    mov $0x1, %rax              # it's ours.
    ret
.spl_mine_end:

