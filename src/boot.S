# Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
# This file is part of the 'tachyon' operating system.

#include <defs.h>
#include <asm.h>
#include <reg.h>
#include <msr.h>

.global bootstrap_x86_64
.global bootstrap_x86_64_x64

.type bootstrap_x86_64, @function
.type bootstrap_x86_64_x64, @function

.size bootstrap_x86_64, .bootstrap_x86_64_end - bootstrap_x86_64
.size bootstrap_x86_64_x64, .bootstrap_x86_64_x64_end - bootstrap_x86_64_x64

# .----------------------------------------.
# | 32 bit init code. switches to 64-bit.  |
# '----------------------------------------'

SECTION_BOOT_TEXT
.code32

.set ALIGN,         1<<0             # align loaded modules on page boundaries
.set MEMINFO,       1<<1             # provide memory map
.set KLUDGE,        1<<16            # use a.out kludge (allows loading of section headers, etc.)
.set FLAGS,         MEMINFO | KLUDGE | ALIGN
.set MAGIC,         0x1BADB002
.set CHECKSUM,      -(MAGIC + FLAGS)

# multiboot header goes here, not into data section,
# since the header needs to be somewhere in the front,
# and the code too.

.align 4
bootstrap_x86_64_mbhdr:
    .long MAGIC                     # the magic number
    .long FLAGS                     # the loader flags
    .long CHECKSUM                  # checksum
    .long bootstrap_x86_64_mbhdr    # address of this header
    .long CORE_LMA_START            # start of the kernel
    .long _core_lma_user_edata      # end of data. kernel (file!) must be at least this big.
    .long _core_lma_ebss            # end of bss, which might not physically exist in file.
    .long bootstrap_x86_64          # entry point address

bootstrap_x86_64:
    # first, let's start witch setting up a stack in lower half.
    mov $bootstrap_x86_64_stack_top, %esp

    # save the boot-loader-set eax & ebx
    mov %eax, (bootstrap_x86_64_boot_eax)
    mov %ebx, (bootstrap_x86_64_boot_ebx)

    lgdt (x86_64_gdt_ptr)        # load the new 64-bit gdt.

    ljmp $0x8,$1f
1:

    mov $0x10, %ax              # set propper data segment selectors.
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    # disable the good ole 8259A PIC, we'll use the APIC instead
    mov $0xFF, %al
    out %al, $0xA1
    out %al, $0x21

    # Assume, that paging structures are correct here, we
    # only need to switch to long mode, and enable paging

    mov %cr4, %eax              # read CR4
    or $CR4_PAE, %eax           # set the PAE bit.
    mov %eax, %cr4              # and write it back.

    mov $IA32_EFER, %ecx        # read extended feature enabled register
    rdmsr                       # do it.
    bts  $8, %eax               # set the LME bit.
    wrmsr                       # and write it back.

    mov $x86_64_pg_pml4, %eax   # load the boot paging PML4
    mov %eax, %cr3              # set the new PML4
    mov %cr0, %eax              # read CR0
    or $CR0_PAGING, %eax        # set the paging enabled bit.
    or $CR0_WRITE_PROTECT, %eax # enable write protection system wide for W/P pages.
    mov %eax, %cr0              # and write it back

    ljmp $0x18,$bootstrap_x86_64_x64

.bootstrap_x86_64_end:

# .----------------------------------------.
# | 64 bit init code. enables paging.      |
# '----------------------------------------'

.code64
bootstrap_x86_64_x64:
    mov %cr4, %rax              # again, load CR4
    or $CR4_GLOBAL_PAGES, %rax  # must be done after setting CR0_PAGING!
    mov %rax, %cr4              # and do it.

    movq $bootstrap_x86_64_stack_top, %rsp
    movq $CORE_VMA_X86_64, %rax
    addq %rax, %rsp

    mov $0x20, %ax              # set propper data segment selectors.
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    mov %ax, %ss

    pushq $0x0
    pushq $0x0
    movq %rsp, %rbp

    call bss_init

    call x86_64_idt_init

    # restore the multiboot-loader information to the correct register
    movl (bootstrap_x86_64_boot_eax), %eax
    mov  %eax, (boot_state)
    movl (bootstrap_x86_64_boot_ebx), %ebx
    mov  %ebx, (boot_state + 4)

    call boot
    call abort

    1:
        cli
        hlt
        jmp 1b

.bootstrap_x86_64_x64_end:

# .----------------------------------------.
# | Data required in lower half.           |
# '----------------------------------------'

SECTION_BOOT_DATA

.align 0x100
bootstrap_x86_64_stack:
    .space 0x10000
    bootstrap_x86_64_stack_top:

bootstrap_x86_64_boot_eax:
    .long 0x0

bootstrap_x86_64_boot_ebx:
    .long 0x0

