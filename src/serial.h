/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include <tachyon.h>

#define PORT_COM1   0x3F8
#define PORT_COM2   0x2F8
#define PORT_COM3   0x3E8
#define PORT_COM4   0x2E8

/**
 * initializes the serial port logging on COM1 
 */
void serial_log_init();

/**
 * Initializes a specific serial port. use the PORT_COM* constants!
 *
 * @param port  the port to initialize
 */
void init_port(uint16_t port);

/**
 * Explicit serial port writing for debugging purposes.
 *
 * @param buf   data to write
 * @param len   size of data to write
 * @param port  port to write data to. use PORT_COM* constants!
 */
void serial_write(void* buf, size_t len, uint16_t port);
