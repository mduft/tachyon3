/* copyright (c) 2011 by markus duft <mduft@gentoo.org>
 * this file is part of the 'tachyon' operating system. */

#include "cpu.h"
#include "thread.h"
#include "dyngdt.h"
#include "tss.h"

#include <extp.h>
#include <kheap.h>
#include <mem.h>
#include <log.h>
#include <string.h>

#include <x86/gdt.h>
#include <x86/lapic.h>

void cpu_init() {
    /* create and set an initial dummy context, which will be 
       released on the first context switch. */
    x86_64_ctx_set(kheap_alloc(sizeof(thr_context_t)));

    dyngdt_init_and_lock();

    dyngdt_set(GDT_KCODE64, 0, 0xFFFFFFFF, GDT_TYPE_CODE | GDT_TYPE_CODE_READ_ENABLE, 0, true, false);
    dyngdt_set(GDT_KDATA64, 0, 0xFFFFFFFF, GDT_TYPE_DATA_WRITE_ENABLE, 0, true, false);

    tss_init();

    dyngdt_activate_and_unlock();

    /* now set the IST bits in the IDT (currently done in idt.S) */

    /* finally, activate the local APIC */
    lapic_init();

    /* give some information on what we're on. */
    cpuid_leaf_t leaf;

    leaf = cpuid(CPUID_01H);
    info("cpu %d: family: %d, model: %d, stepping: %d\n", lapic_cpuid(),
        CPUID_01H_V_FAMILY(leaf), CPUID_01H_V_MODEL(leaf), CPUID_01H_V_STEPPING(leaf));
    info("cpu %d: extended family: %d, extended model: %d\n", lapic_cpuid(),
        CPUID_01H_V_EX_FAMILY(leaf), CPUID_01H_V_EX_MODEL(leaf));

    leaf = cpuid(CPUID_8_0H);
    uint32_t max_leaf = CPUID_8_0H_MAX_EXTID(leaf);

    if(max_leaf >= CPUID_8_04H) {
        union {
            char buf[52];
            uint32_t data[13];
        } u;
        size_t i = 0;
        memset(u.buf, 0, sizeof(u.buf));

        #define set_leaf \
            u.data[i++] = leaf.ax; \
            u.data[i++] = leaf.bx; \
            u.data[i++] = leaf.cx; \
            u.data[i++] = leaf.dx;

        leaf = cpuid(CPUID_8_02H);
        set_leaf
        leaf = cpuid(CPUID_8_03H);
        set_leaf
        leaf = cpuid(CPUID_8_04H);
        set_leaf

        info("cpu %d: %s\n", lapic_cpuid(), u.buf);
    }
}

cpuid_leaf_t cpuid(uint32_t leaf) {
    cpuid_leaf_t res;

    asm volatile("cpuid" : "=a"(res.ax), "=b"(res.bx), "=c"(res.cx), "=d"(res.dx) : "a"(leaf));

    return res;
}

INSTALL_EXTENSION(EXTP_CPUINIT, cpu_init, "cpu state");

