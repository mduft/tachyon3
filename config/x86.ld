OUTPUT_FORMAT("elf32-i386")
OUTPUT_ARCH("i386")

ENTRY(bootstrap_x86)

#include <x86/defs.h>

CORE_LMA_START = CORE_LMA_X86;
CORE_VMA_OFF   = CORE_VMA_X86;

#include "common-layout.ld"
