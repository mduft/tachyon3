/* Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
 * This file is part of the 'tachyon' operating system. */

OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64", "elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)

ENTRY(bootstrap_x86_64)

#include <defs.h>

CORE_LMA_START = CORE_LMA_X86_64;
CORE_VMA_OFF   = CORE_VMA_X86_64;
UAPI_VMA       = UAPI_VMA_X86_64;

#include "common-layout.ld"

_x86_64_idt_vma = x86_64_idt + CORE_VMA_X86_64;
