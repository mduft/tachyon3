# Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
# This file is part of the 'tachyon' operating system.

#include <defs.h>
#include <asm.h>

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
# |  * Thread SS            (0x50)          |
# |  * Thread RSP           (0x48)          |
# |  * Thread EFLAGS        (0x40)          |
# |  * Thread CS            (0x38)          |
# |  * Thread EIP           (0x30)          |
# |  * Intr Error Code      (0x28)          |
# |  * Intr Number          (0x20)          |
# |  * Thread State Storage (0x18)          |
# |  * ISR Return Address   (0x10)          |
# |  + Thread RAX           (0x08)          |
# |  + Thread RBX           (0x00)          |
# |                                         |
# | (Those with '+' are pushed during save  |
# |  and restore)                           |
# '-----------------------------------------'

.section .text

x86_64_isr_state_save:
    push %rax               # save original RAX, used during save.
    push %rbx               # save original RBX, used during save.

    mov 0x18(%rsp), %rax    # load address of the state structure.

                            # .---------------------------.
                            # | layout of state structure |
                            # | has to match this code!   |
    mov 0x30(%rsp), %rbx    # |                    (load) |
    mov %rbx, (%rax)        # |  * RIP   (0x00)    (save) |
    mov 0x38(%rsp), %rbx    # |                    (load) |
    mov %rbx, 0x08(%rax)    # |  * CS    (0x08)    (save) |
    mov 0x40(%rsp), %rbx    # |                    (load) |
    mov %rbx, 0x10(%rax)    # |  * RFL   (0x10)    (save) |
    mov 0x48(%rsp), %rbx    # |                    (load) |
    mov %rbx, 0x18(%rax)    # |  * RSP   (0x18)    (save) |
    mov 0x50(%rsp), %rbx    # |                    (load) |
    mov %rbx, 0x20(%rax)    # |  * SS    (0x20)    (save) |
    mov %rbp, 0x28(%rax)    # |  * RBP   (0x28)    (save) |
    mov 0x08(%rsp), %rbx    # |                    (load) |
    mov %rbx, 0x30(%rax)    # |  * RAX   (0x30)    (save) |
    mov (%rsp), %rbx        # |                    (load) |
    mov %rbx, 0x38(%rax)    # |  * RBX   (0x38)    (save) |
    mov %rcx, 0x40(%rax)    # |  * RCX   (0x40)    (save) |
    mov %rdx, 0x48(%rax)    # |  * RDX   (0x48)    (save) |
    mov %rsi, 0x50(%rax)    # |  * RSI   (0x50)    (save) |
    mov %rdi, 0x58(%rax)    # |  * RDI   (0x58)    (save) |
    mov %r8,  0x60(%rax)    # |  * R8    (0x60)    (save) |
    mov %r9,  0x68(%rax)    # |  * R9    (0x68)    (save) |
    mov %r10, 0x70(%rax)    # |  * R10   (0x70)    (save) |
    mov %r11, 0x78(%rax)    # |  * R11   (0x78)    (save) |
    mov %r12, 0x80(%rax)    # |  * R12   (0x80)    (save) |
    mov %r13, 0x88(%rax)    # |  * R13   (0x88)    (save) |
    mov %r14, 0x90(%rax)    # |  * R14   (0x90)    (save) |
    mov %r15, 0x98(%rax)    # |  * R15   (0x98)    (save) |
    mov %cr0, %rbx          # |                    (load) |
    mov %rbx, 0xa0(%rax)    # |  * CR0   (0xa0)    (save) |
    mov %cr2, %rbx          # |                    (load) |
    mov %rbx, 0xa8(%rax)    # |  * CR2   (0xa8)    (save) |
    mov %cr3, %rbx          # |                    (load) |
    mov %rbx, 0xb0(%rax)    # |  * CR3   (0xb0)    (save) |
    mov %cr4, %rbx          # |                    (load) |
    mov %rbx, 0xb8(%rax)    # |  * CR4   (0xb8)    (save) |
    mov %cr8, %rbx          # |                    (load) |
    mov %rbx, 0xc0(%rax)    # |  * CR8   (0xc0)    (save) |
                            # |                           |
                            # | TODO: %fs, %gs?           |
                            # '---------------------------'

    pop %rbx
    pop %rax

    ret
.x86_64_isr_state_save_end:

x86_64_isr_state_restore:
    push %rax               # save original RAX, used during restore (will be replaced with correct value!).
    push %rbx               # save original RBX, used during restore (will be replaced with correct value!).

    mov 0x18(%rsp), %rax    # load address of the state structure.

    # the following does the opposite of the save function above.
    # each field is read from the structure, and written back into
    # registers or on the stack.

                            # .----------------.
    mov (%rax), %rbx        # |         (load) |
    mov %rbx, 0x30(%rsp)    # | * RIP   (save) |
    mov 0x08(%rax), %rbx    # |         (load) |
    mov %rbx, 0x38(%rsp)    # | * CS    (save) |
    mov 0x10(%rax), %rbx    # |         (load) |
    mov %rbx, 0x40(%rsp)    # | * RFL   (save) |
    mov 0x18(%rax), %rbx    # |         (load) |
    mov %rbx, 0x48(%rsp)    # | * RSP   (save) |
    mov 0x20(%rax), %rbx    # |         (load) |
    mov %rbx, 0x50(%rsp)    # | * SS    (save) |
    mov 0x28(%rax), %rbp    # | * RBP   (save) |
    mov 0x30(%rax), %rbx    # |         (load) |
    mov %rbx, 0x08(%rsp)    # | * RAX   (save) |
    mov 0x38(%rax), %rbx    # |         (load) |
    mov %rbx, (%rsp)        # | * RBX   (save) |
    mov 0x40(%rax), %rcx    # | * RCX   (save) |
    mov 0x48(%rax), %rdx    # | * RDX   (save) |
    mov 0x50(%rax), %rsi    # | * RSI   (save) |
    mov 0x58(%rax), %rdi    # | * RDI   (save) |
    mov 0x60(%rax), %r8     # | * R8    (save) |
    mov 0x68(%rax), %r9     # | * R9    (save) |
    mov 0x70(%rax), %r10    # | * R10   (save) |
    mov 0x78(%rax), %r11    # | * R11   (save) |
    mov 0x80(%rax), %r12    # | * R12   (save) |
    mov 0x88(%rax), %r13    # | * R13   (save) |
    mov 0x90(%rax), %r14    # | * R14   (save) |
    mov 0x98(%rax), %r15    # | * R15   (save) |
                            # '----------------'

    pop %rbx                # pop new value for rbx.
    pop %rax                # pop new value for rax.
    ret
.x86_64_isr_state_restore_end:

