/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include <tachyon.h>

/**
 * Initializes the serial port for logging.
 */
void serial_init();

/**
 * Extension point to write string over the serial line.
 *
 * @param str   the string to write
 */
void serial_write(char const* str);
