/* copyright (c) 2010 by markus duft <mduft@gentoo.org>
 * this file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"
#include "pmem.h"
#include "rd.h"

/**
 * Tries to find an initial RAM disc from the multiboot header
 * passed to the kernel upon boot
 */
rd_header_t* mboot_find_rd();
