/* Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
 * This file is part of the 'tachyon' operating system. */

#include "serial.h"
#include <extp.h>
#include <log.h>
#include <io.h>

void serial_port_init(uint16_t port) {
    outb(0x00, port + 1);   // Disable all interrupts
    outb(0x80, port + 3);   // Enable DLAB (set baud rate divisor)
    outb(0x03, port + 0);   // Set divisor to 3 (lo byte) 38400 baud
    outb(0x00, port + 1);   //                  (hi byte)
    outb(0x03, port + 3);   // 8 bits, no parity, one stop bit
    outb(0xC7, port + 2);   // Enable FIFO, clear them, with 14-byte threshold
    outb(0x0B, port + 4);   // IRQs enabled, RTS/DSR set
}

void serial_write(void* buf, size_t len, uint16_t port) {
    for(size_t bytes = 0; bytes < len; ++bytes) {
        while ((inb(port + 5) & 0x20) == 0);
        outb(((char*)buf)[bytes], port);
    }
}

static void serial_log_com1(char const* str) {
    while(str && *str) {
        while ((inb(PORT_COM1 + 5) & 0x20) == 0);
        outb(*str++, PORT_COM1);
    }
}

void serial_log_init() {
    serial_port_init(PORT_COM1);

    log_add_writer(serial_log_com1, "serial-log");
    log_set_level("serial-log", Trace);
}

