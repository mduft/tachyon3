# Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
# This file is part of the 'tachyon' operating system.

#include <x86_64/defs.h>
#include <x86/asm.h>

.global x86_64_isr_state_save
.global x86_64_isr_state_restore

.type x86_64_isr_state_save, @function
.type x86_64_isr_state_restore, @function

.size x86_64_isr_state_save, .x86_64_isr_state_save_end - x86_64_isr_state_save
.size x86_64_isr_state_restore, .x86_64_isr_state_restore_end - x86_64_isr_state_restore

# .-----------------------------------------.
# | Saving and restoring of cpu state       |
# |                                         |
# | The stack before and after restoring    |
# | should look like this:                  |
# |  * Thread SS            (0x58) (0x40)   |
# |  * Thread RSP           (0x50) (0x38)   |
# |  * Thread EFLAGS        (0x48) (0x30)   |
# |  * Thread CS            (0x40) (0x28)   |
# |  * Thread EIP           (0x38) (0x20)   |
# |  * Intr Error Code      (0x30) (0x18)   |
# |  * Intr Number          (0x28) (0x10)   |
# |  * Thread State Storage (0x20) (0x08)   |
# |  * ISR Return Address   (0x18) (0x00)   |
# |  + Thread RAX           (0x10)          |
# |  + Thread RBX           (0x08)          |
# |  + Thread RFLAGS        (0x00)          |
# |                                         |
# | (Those with '+' are pushed during save) |
# '-----------------------------------------'

.section .text

x86_64_isr_state_save:
    push %rax               # save original RAX, used during save.
    push %rbx               # save original RBX, used during save.
    pushf                   # save original flags, possibly influenced here.

    mov 0x20(%rsp), %rax    # load address of the state structure.
    mov 0x28(%rsp), %rbx    # load RIP from the stack.
    mov %rbx, (%rax)        # save RIP to the struct.

    pop %rbx
    pop %rbx
    pop %rbx

    ret
.x86_64_isr_state_save_end:

x86_64_isr_state_restore:
    # restores the above stack layout (except those pushed during save).
.x86_64_isr_state_restore_end:

