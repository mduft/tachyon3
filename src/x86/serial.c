/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "serial.h"
#include <extp.h>
#include <log.h>
#include <io.h>

INSTALL_EXTENSION(EXTP_KINIT, serial_init, "serial-log")

#define PORT_COM1   0x3F8

void serial_init() {
    outb(0x00, PORT_COM1 + 1);   // Disable all interrupts
    outb(0x80, PORT_COM1 + 3);   // Enable DLAB (set baud rate divisor)
    outb(0x03, PORT_COM1 + 0);   // Set divisor to 3 (lo byte) 38400 baud
    outb(0x00, PORT_COM1 + 1);   //                  (hi byte)
    outb(0x03, PORT_COM1 + 3);   // 8 bits, no parity, one stop bit
    outb(0xC7, PORT_COM1 + 2);   // Enable FIFO, clear them, with 14-byte threshold
    outb(0x0B, PORT_COM1 + 4);   // IRQs enabled, RTS/DSR set

    log_add_writer(serial_write, "serial-log");
    log_set_level("serial-log", Trace);
}

void serial_write(char const* str) {
    while(str && *str) {
        while ((inb(PORT_COM1 + 5) & 0x20) == 0);
        outb(*str++, PORT_COM1);
    }
}

