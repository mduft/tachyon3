/* Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#define CGA_WIDTH      80
#define CGA_HEIGHT     25

#define CGA_VR_LOCATION   CGA_VIRTUAL
#define CGA_VR_PHYSICAL   0xB8000
#define CGA_VR_SIZE       (CGA_WIDTH*CGA_HEIGHT*2)

/**
 * Initializes and clears the CGA screen. This can be used
 * to clear the screen, if need be.
 */
void cga_init();

/**
 * Write a string to the CGA adapter. The screen position
 * is calculated automatically, and the screen will also
 * automatically scroll down, when the bottom is reached.
 *
 * @param str   string to write.
 */
void cga_write(char const* str);

