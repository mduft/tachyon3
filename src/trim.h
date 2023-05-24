/* copyright (c) 2023 by markus duft <markus.duft@ssi-schaefer.com>
 * this file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

/**
 * Trims a given string on the left, removing all whitespace.
 * 
 * The result pointer is equal to the input, so it can be used in
 * free normally.
 */
char* ltrim(char *const s);

/**
 * Trims a given string on the right, removing all whitespace.
 *
 * The original string is modified by inserting a '\0' at the
 * determined new end of the string.
 */
char* rtrim(char *const s);

/**
 * A combination of ltrim and rtrim to trim whitespaces on both sides
 * of a string. Restrictions documented with ltrim and rtrim apply.
 */
char* trim(char *const s);

