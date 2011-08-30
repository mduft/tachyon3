/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

/**
 * Initializes the generic page-fault handler that is able of
 * dynamically growing stacks.
 */
void pgflt_init();
