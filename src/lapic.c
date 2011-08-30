/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "lapic.h"
#include "msr.h"
#include "paging.h"

#include <pmem.h>
#include <log.h>
#include <vmem.h>
#include <intr.h>
#include <spc.h>
#include <intr.h>

static phys_addr_t lapic_phys() {
    uint64_t apic = msr_read(IA32_APIC_BASE);
    /* base address is a 24-bit field only! */
    return (apic & ~0xFFF);
}

bool lapic_is_enabled() {
    uint64_t apic = msr_read(IA32_APIC_BASE);
    return (apic & APIC_GLOBAL_ENABLE && (APIC_REG(APIC_REG_SV) & APIC_SV_ENABLE));
}

static bool lapic_handle_error(interrupt_t* state) {
    error("local APIC error!\n");
    lapic_eoi();
    return true;
}

static bool lapic_handle_spurious(interrupt_t* state) {
    error("spurious interrupt!\n");
    lapic_eoi();
    return true;
}

void lapic_init() {
    if(intr_state())
        fatal("interrupts may not be enabled in lapic_init()\n");

    // another CPU may have done this already!
    phys_addr_t mapping = vmem_resolve(spc_current(), (void*)APIC_VIRTUAL);
    if(mapping != 0 && mapping != lapic_phys())
        fatal("local apic virtual mapping not available!\n");

    if(!mapping) {
        // try to reserve, and ignore if it is already (other cpus use the same address).
        // this relies on the physical region not yet being touched by the allocator.
        if(!pmem_reserve(lapic_phys(), APIC_VSZ))
            fatal("failed to reserve physical APIC region\n");

        if(!vmem_map(spc_current(), lapic_phys(), (void*)APIC_VIRTUAL, 
                PG_WRITABLE | PG_NONCACHABLE | PG_WRITETHROUGH | PG_GLOBAL))
            fatal("failed to map APIC memory\n");
    }

    intr_add(IRQ_SPURIOUS, lapic_handle_spurious);
    intr_add(IRQ_ERROR, lapic_handle_error);

    APIC_REG(APIC_REG_SV)           |= IRQ_SPURIOUS | APIC_SV_ENABLE;
    APIC_REG(APIC_REG_LVT_ERROR)    |= IRQ_ERROR;

    /* requires back-to-back write */
    APIC_REG(APIC_REG_ERROR_STATUS) = 0;
    APIC_REG(APIC_REG_ERROR_STATUS) = 0;

    APIC_REG(APIC_REG_TPR)          = 0;
    APIC_REG(APIC_REG_EOI)          = 0;

    APIC_REG(APIC_REG_TPR)          = 0;

    if(!lapic_is_enabled())
        fatal("failed to enable the local APIC on CPU %d\n", lapic_cpuid());

    info("local APIC 0x%x on CPU %d enabled at physical %p\n", (APIC_REG(APIC_REG_VERSION) & 0x7F), lapic_cpuid(), lapic_phys());
}

cpuid_t lapic_cpuid() {
    // TODO: x2APIC uses the whole field as id!
    return (APIC_REG(APIC_REG_ID) >> 24);
}

void lapic_eoi() {
    APIC_REG(APIC_REG_EOI) = 0;
}

bool lapic_is_bsp() {
    uint64_t apic = msr_read(IA32_APIC_BASE);
    return (apic & APIC_BSP);
}
