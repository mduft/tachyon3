/* Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"
#include "extp.h"

extern uintptr_t _core_lma_start;
extern uintptr_t _core_vma_start;
extern uintptr_t _core_vma_edata;
extern uintptr_t _core_lma_edata;
extern uintptr_t _core_vma_bss;
extern uintptr_t _core_vma_ebss;
extern uintptr_t _core_lma_ebss;
extern uintptr_t _core_vma_end;
extern uintptr_t _core_lma_end;

extern uintptr_t _core_vma_user_code;
extern uintptr_t _core_lma_user_code;
extern uintptr_t _core_vma_user_ecode;
extern uintptr_t _core_lma_user_ecode;
extern uintptr_t _core_vma_user_data;
extern uintptr_t _core_lma_user_data;
extern uintptr_t _core_vma_user_edata;
extern uintptr_t _core_lma_user_edata;

extern extension_point_t* _core_vma_sextp;
extern extension_point_t* _core_vma_eextp;

extern uintptr_t _x86_64_idt_vma;
