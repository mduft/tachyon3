/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"
#include "extp.h"

extern uintptr_t _core_lma_start;
extern uintptr_t _core_vma_start;
extern uintptr_t _core_vma_edata;
extern uintptr_t _core_lma_edata;
extern uintptr_t _core_vma_ebss;
extern uintptr_t _core_lma_ebss;
extern uintptr_t _core_vma_end;
extern uintptr_t _core_lma_end;

extern extension_point_t* _core_vma_sextp;
extern extension_point_t* _core_vma_eextp;
