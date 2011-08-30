/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "ioapic.h"
#include "paging.h"

#include <extp.h>
#include <log.h>
#include <pmem.h>
#include <vmem.h>
#include <spc.h>

typedef struct {
    uint32_t regsel;
    uint32_t pad[3];
    uint32_t iowin;
} PACKED ioapic_t;

static ioapic_t* _the_ioapic;

static void ioapic_write(ioapic_t* ioapic, uint32_t reg, uint32_t data) {
    ioapic->regsel = reg;
    ioapic->iowin = data;
}

static uint32_t ioapic_read(ioapic_t* ioapic, uint32_t reg) {
    ioapic->regsel = reg;
    return ioapic->iowin;
}

static void ioapic_init() {
    // TODO: multiple ioapics, etc.
    if(!pmem_reserve(IOAPIC_BASE, IOAPIC_VSZ))
        fatal("failed to reserve physical I/O APIC memory!\n");

    if(!vmem_map(spc_current(), IOAPIC_BASE, (void*)IOAPIC_VIRTUAL,
            PG_WRITABLE | PG_NONCACHABLE | PG_WRITETHROUGH | PG_GLOBAL))
        fatal("failed to map I/O APIC memory!\n");

    _the_ioapic = (ioapic_t*)IOAPIC_VIRTUAL;

    uint32_t idreg = ioapic_read(_the_ioapic, IOAPIC_REG_ID);
    uint32_t vreg = ioapic_read(_the_ioapic, IOAPIC_REG_VER);

    /* initially clear all redirection entries. resets all
     * entries to edge-triggered, active high, disabled, and
     * not routed to any cpu */
    for(uint32_t i = 0; i <= IOAPIC_MAX_REDIR(vreg); ++i) {
        ioapic_write(_the_ioapic, IOAPIC_REG_TABLE + (2 * i), IOAPIC_INT_MASKED | (IRQ_NUM(i)));
        ioapic_write(_the_ioapic, IOAPIC_REG_TABLE + ((2 * i) + 1), 0);
    }

    info("i/o apic %d version: 0x%x, max irq redirections: %d\n",
        IOAPIC_ID(idreg), IOAPIC_VER(vreg), IOAPIC_MAX_REDIR(vreg));
}

INSTALL_EXTENSION(EXTP_PLATFORM_INIT, ioapic_init, "i/o apic");

void ioapic_enable(uint8_t num, uint32_t cpuid) {
    info("enable irq %d (int %d) on cpu %d\n", num, IRQ_NUM(num), cpuid);
    ioapic_write(_the_ioapic, IOAPIC_REG_TABLE + (2 * num), (IRQ_NUM(num)));
    ioapic_write(_the_ioapic, IOAPIC_REG_TABLE + ((2 * num) + 1), cpuid << 24);
}

void ioapic_disable(uint8_t num, uint32_t cpuid) {
    info("disable irq %d (int %d) on cpu %d\n", num, IRQ_NUM(num), cpuid);
    ioapic_write(_the_ioapic, IOAPIC_REG_TABLE + (2 * num), IOAPIC_INT_MASKED | (IRQ_NUM(num)));
    ioapic_write(_the_ioapic, IOAPIC_REG_TABLE + ((2 * num) + 1), 0);
}

