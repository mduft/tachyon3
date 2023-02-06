/* Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

#define MAX_CMD_ARGUMENTS 1024

/**
 * Initializes the kernel command line. This gathers information from
 * the EXTP_CMD_LINE extension point. Multiple extension points could
 * contribute to the final kernel command line.
 */
void cmd_init();

/**
 * Adds an argument to the existing kernel command line.
 */
void cmd_add(char const* argument);

/**
 * Retrieves the kernel command line as provided by the extensions.
 *
 * The return value is a list of separate strings forming the arguments.
 */
char const** cmd_get();

