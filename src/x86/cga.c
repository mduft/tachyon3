/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "cga.h"
#include <extp.h>
#include <vmem.h>
#include <spc.h>
#include <log.h>
#include "paging.h"

INSTALL_EXTENSION(EXTP_VMEM_INIT, cga_init, "screen")

static uint16_t __cga_x = 0;
static uint16_t __cga_y = 0;
static uint16_t __cga_attr = 0x0700;

void cga_init() {
    vmem_map(spc_current(), CGA_VR_PHYSICAL, (void*)CGA_VR_LOCATION, PG_WRITABLE);

    asm("cld; rep stosl;"
        :   /* no output */
        :   "D"(CGA_VR_LOCATION),
            "c"(CGA_VR_SIZE / 4),
            "a"(0x0F200F20)
        );

    /* hide the cursor */
    asm("outb %%al, %%dx;"
        :   /* no output */
        :   "a"(0xA),
            "d"(0x3D4)
        );
    asm("outb %%al, %%dx;"
        :   /* no output */
        :   "a"(0x20),
            "d"(0x3D5)
        );

    __cga_x = __cga_y = 0;

    log_add_writer(cga_write, "screen-log");
}

static void scroll1() {
    register int16_t* vram_source = (int16_t*)CGA_VR_LOCATION + CGA_WIDTH;
    register int16_t* vram_dest = (int16_t*)CGA_VR_LOCATION;
    register intmax_t move_len = (CGA_WIDTH * CGA_HEIGHT) - CGA_WIDTH;

    while(move_len--) {
        *vram_dest++ = *vram_source++;
    }

    /* clear the last line. vram_dest points there */
    move_len = CGA_WIDTH;
    while(move_len--) {
        *vram_dest++ = ' ' | __cga_attr;
    }
}

#define CALC_LOCATION (int16_t*)CGA_VR_LOCATION + (((CGA_WIDTH) * __cga_y) + __cga_x)

void cga_write(char const* str) {
    int16_t* vram_current = CALC_LOCATION;

    while(str && *str) {
        if(*str != '\n' && *str != '\r') {
            *vram_current = *str | __cga_attr;
            ++__cga_x;
        }

        if(__cga_x >= CGA_WIDTH || *str == '\n' || *str == '\r') {
            __cga_x = 0;
            if(*str != '\r') {
                ++__cga_y;
                vram_current = CALC_LOCATION - 1;
            }
        }

        if(__cga_y >= CGA_HEIGHT) {
            scroll1();
            __cga_y = CGA_HEIGHT - 1;
            vram_current = CALC_LOCATION - 1;
        }

        ++str;
        ++vram_current;
    }
}
