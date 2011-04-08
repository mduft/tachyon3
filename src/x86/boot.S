# Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
# This file is part of the 'tachyon' operating system.

#include <x86/defs.h>
#include <x86/asm.h>
#include <x86/reg.h>
#include <x86/msr.h>

.global bootstrap_x86
.type bootstrap_x86, @function
.size bootstrap_x86, .bootstrap_x86_end - bootstrap_x86

# .------------------------------.
# | DATA Section                 |
# '------------------------------'

.section .data

.align 0x1000
# a whopping 64K of initial stack space.
x86_core_stack:
    .space 0x10000, 0x00
    x86_core_stack_top:

# .------------------------------.
# | TEXT Section                 |
# '------------------------------'

SECTION_BOOT_TEXT
.code32

.set ALIGN,         1<<0             # align loaded modules on page boundaries
.set MEMINFO,       1<<1             # provide memory map
.set KLUDGE,        1<<16            # use a.out kludge (allows loading of section headers, etc.)
.set FLAGS,         ALIGN | MEMINFO  # this is the Multiboot 'flag' field
.set MAGIC,         0x1BADB002       # 'magic number' lets bootloader find the header
.set CHECKSUM,      -(MAGIC + FLAGS) # checksum required

.align 4
x86_mbhdr:
    .long MAGIC
    .long FLAGS
    .long CHECKSUM
    .long x86_mbhdr

bootstrap_x86:
    # setup the stack
    mov   $x86_core_stack_top, %esp

    # adjust address to be physical, stack is in data segment, which is linked to
    # the kernels virtual higher half address.
    subl  $CORE_VMA_X86, %esp

    # setup a 32 bit gdt.
    lgdt  (x86_gdt_ptr)

    ljmp $0x8,$1f
1:

    push %eax
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

    # setup boot paging to map kernel to higher half

    #mov %cr4, %eax               # read CR4
    #or $CR4_PAE, %eax            # set the PAE bit.
    #mov %eax, %cr4               # and write it back.

    mov $x86_pg_pd, %eax         # load 32 bit PD
    mov %eax, %cr3               # and activate it.
    mov %cr0, %eax               # get current cr0
    or $CR0_PAGING, %eax         # set paging bit
    or $CR0_WRITE_PROTECT, %eax  # enable global write protection
    mov %eax, %cr0               # and set the new value

    mov %cr4, %eax               # again, load CR4
    or $CR4_GLOBAL_PAGES, %eax   # must be done after setting CR0_PAGING!
    mov %eax, %cr4               # and do it.
    pop %eax

    # adjust stack registers to point to the now mapped stack as virtual address
    addl  $CORE_VMA_X86, %esp

    # terminate stack with zero return address, and zero base pointer.
    push $0x0
    push $0x0

    mov   %esp, %ebp

    # .------------------------------------.
    # | Main kernel entry (C/C++)          |
    # '------------------------------------'

    push %eax
    push %ebx

    call bss_init

    pop %eax
    pop %ebx

    # store the initial boot state
    mov   %eax, (boot_state + 4)
    mov   %ebx, (boot_state)

    call x86_idt_init

    call boot

    # .------------------------------------.
    # | the end... this should prolly never|
    # | happen if the kernel behaves...    |
    # '------------------------------------'

    call abort
.bootstrap_x86_end:
