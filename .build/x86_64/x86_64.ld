


OUTPUT_FORMAT("elf64-x86-64", "elf64-x86-64", "elf64-x86-64")
OUTPUT_ARCH(i386:x86-64)

ENTRY(bootstrap_x86_64)
       


CORE_LMA_START = 0x0000000000100000;
CORE_VMA_OFF = 0xFFFFFFFF80000000;
SECTIONS {
    . = CORE_LMA_START + SIZEOF_HEADERS;
    _core_lma_start = .;
    _core_vma_start = . + CORE_VMA_OFF;
    .bootstrap : {
        *(.boot_text)
        *(.boot_data)
    }
    . += CORE_VMA_OFF;
    .text ALIGN(0x1000) : AT(ADDR(.text) - CORE_VMA_OFF) {
        *(.text*)
        . = ALIGN(0x8);
        *(.rodata*)
    }
    .data ALIGN(0x1000) : AT(ADDR(.data) - CORE_VMA_OFF) {
        *(.data)
        _core_vma_edata = .;
        _core_lma_edata = . - CORE_VMA_OFF;
    }
    .bss ALIGN(0x1000) : AT(ADDR(.bss) - CORE_VMA_OFF) {
        *(COMMON)
        *(.bss)
        . = ALIGN(0x1000);
        _core_vma_ebss = .;
        _core_lma_ebss = . - CORE_VMA_OFF;
    }
    _core_vma_end = .;
    _core_lma_end = . - CORE_VMA_OFF;
    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
    }
}
