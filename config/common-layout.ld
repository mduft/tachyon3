/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

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

        /* those need to be aligned at least to 16 byte boundaries */
        . = ALIGN(0x10);
        _core_vma_sextp = .;
        *(.extp)
        _core_vma_eextp = .;
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

    /* the actual kernel ended here, coming up is the attached usermode
     * code that is required to be able to use trampolines and such
     */

    UAPI_VMA_OFF = UAPI_VMA - _core_lma_end;
    . = UAPI_VMA;

    .user.c ALIGN(0x1000) : AT(ADDR(.user.c) - UAPI_VMA_OFF) {
        _core_vma_user_code = .;
        _core_lma_user_code = . - UAPI_VMA_OFF;

        *(.user.c)
        . = ALIGN(0x1000);

        _core_vma_user_ecode = .;
        _core_lma_user_ecode = . - UAPI_VMA_OFF;
    }

    .user.d ALIGN(0x1000) : AT(ADDR(.user.d) - UAPI_VMA_OFF) {
        _core_vma_user_data = .;
        _core_lma_user_data = . - UAPI_VMA_OFF;

        *(.user.d)
        . = ALIGN(0x1000);

        _core_vma_user_edata = .;
        _core_lma_user_edata = . - UAPI_VMA_OFF;
    }

    /DISCARD/ : {
        *(.comment)
        *(.eh_frame)
    }
}
