/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "dyngdt.h"

#include <pmem.h>
#include <vmem.h>
#include <log.h>
#include <spc.h>
#include <mem.h>
#include <extp.h>
#include <spl.h>

#include <x86/gdt.h>
#include <x86/paging.h>

spinlock_t global_gdt_init_lock;

static void dyngdt_init_spinlock() {
    spl_init(&global_gdt_init_lock);
}

INSTALL_EXTENSION(EXTP_EARLY_KINIT, dyngdt_init_spinlock, "gdt lock");

void dyngdt_init_and_lock() {
    spl_lock(&global_gdt_init_lock);
    phys_addr_t pa = pmem_alloc(GDT_VSZ, PAGE_SIZE_4K);
    
    if(!pa)
        fatal("failed to allocate space for dynamic gdt\n");

    if(!vmem_map(spc_current(), pa, (void*)GDT_VIRTUAL, PG_WRITABLE | PG_GLOBAL))
        fatal("mapping the GDT for this cpu failed, already initialized?\n");

    memset((void*)GDT_VIRTUAL, 0, GDT_VSZ);
}

void dyngdt_set(uint16_t sel, uintptr_t base, uint32_t limit, uint32_t type, uint8_t dpl, bool large, bool sys) {
    if(dpl > 2)
        fatal("dpl out of range for gdt entry: %d\n", dpl);

    if(!spl_locked(&global_gdt_init_lock))
        fatal("global gdt initialization lock not locked!\n");

    selector_t selector;
    memset(&selector, 0, sizeof(selector_t));

    selector.fields.limit_low = limit & 0xFFFF;
    selector.fields.base_low = base & 0xFFFF;
    selector.fields.base_m_low = (base >> 16) & 0xFF;
    selector.fields.type = type;
    selector.fields.dpl = dpl;
    selector.fields.present = 1;
    selector.fields.limit_high = (limit >> 16) & 0xF;
    selector.fields.large =(large ? 1 : 0);
    selector.fields.opsz = (large ? 0 : 1); // 16 bit not supported! either 64 (large) or 32bit.
    selector.fields.base_m_high = (base >> 24) & 0xFF;

    if(!sys) {
        selector.fields.seg_sel = 1;
        selector.fields.granularity = 1; // always use 4K limit increments with segment selectors.
    }

    sel /= 8; // offset to index into raw gdt array.
    uint64_t* raw_gdt = (uint64_t*)GDT_VIRTUAL;

    raw_gdt[sel] = selector.bytes;

    if(sys && (type & GDT_TYPE_TSS) != 0) {
        selector_ext_t ext;
        memset(&ext, 0, sizeof(selector_ext_t));

        ext.fields.base_high = (base >> 32);
        raw_gdt[sel+1] = ext.bytes;
    }
}

void dyngdt_activate_and_unlock() {
    if(!spl_locked(&global_gdt_init_lock))
        fatal("global gdt initialization lock not locked!\n");

    struct {
        uint16_t limit;
        uint64_t base;
    } PACKED gdt_ptr = {
        GDT_VSZ, vmem_resolve(spc_current(), (void*)GDT_VIRTUAL)
    };

    asm volatile (
        "\tlgdt %0\n"
        "\tpushq %1\n"
        "\tpushq $1f\n"
        "\tlretq\n"
        "\t1:\n"
        "\tmovq %2, %%rax\n"
        "\tmovq %%rax, %%ss\n"
        "\tmov %3, %%ax\n"
        "\tltr %%ax\n"
        :: "m"(gdt_ptr), "i"(GDT_KCODE64), "i"(GDT_KDATA64), "i"(GDT_KTSS) : "rax");

    vmem_unmap(spc_current(), (void*)GDT_VIRTUAL);
    spl_unlock(&global_gdt_init_lock);
}
