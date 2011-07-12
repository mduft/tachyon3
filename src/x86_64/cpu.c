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

static uint32_t _cpumaxid = 0;
static cpu_locals_t* _cpulcl = NULL;
static spinlock_t _cpulck;
static bool _cpuinit = false;

static thr_context_t _dummy_context;
static cpu_locals_t _dummy_bsplocals = { .context = &_dummy_context };

void cpu_init() {
    if(intr_state())
        fatal("interrupts shall not be enabled in cpu_init!\n");

    dyngdt_init_and_lock();

    dyngdt_set(GDT_KCODE64, 0, 0xFFFFFFFF, GDT_TYPE_CODE | GDT_TYPE_CODE_READ_ENABLE, 0, true, false);
    dyngdt_set(GDT_KDATA64, 0, 0xFFFFFFFF, GDT_TYPE_DATA_WRITE_ENABLE, 0, true, false);

    tss_init();

    dyngdt_activate_and_unlock();

    /* now set the IST bits in the IDT (currently done in idt.S) */

    /* finally, activate the local APIC */
    lapic_init();

    uint32_t id = lapic_cpuid();

    /* allocate room for the CPUs locals. */
    _cpumaxid = max(_cpumaxid, id);
    _cpulcl = kheap_realloc(_cpulcl, sizeof(cpu_locals_t) * _cpumaxid); // TODO: naive; could be better
    memset(&_cpulcl[id], 0, sizeof(cpu_locals_t));

    _cpulcl[id].context = &_dummy_context;

    trace("cpu %d: locals at %p\n", id, &_cpulcl[id]);

    /* give some information on what we're on. */
    cpuid_leaf_t leaf;

    leaf = cpuid(CPUID_01H);
    info("cpu %d: family: %d, model: %d, stepping: %d\n", id,
        CPUID_01H_V_FAMILY(leaf), CPUID_01H_V_MODEL(leaf), CPUID_01H_V_STEPPING(leaf));
    info("cpu %d: extended family: %d, extended model: %d\n", id,
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

        info("cpu %d: %s\n", id, u.buf);
    }
}

void cpu_bsp_init() {
    spl_init(&_cpulck);
    cpu_init();

    _cpuinit = true;
}

INSTALL_EXTENSION(EXTP_CPUINIT, cpu_bsp_init, "cpu state");

cpuid_leaf_t cpuid(uint32_t leaf) {
    cpuid_leaf_t res;

    asm volatile("cpuid" : "=a"(res.ax), "=b"(res.bx), "=c"(res.cx), "=d"(res.dx) : "a"(leaf));

    return res;
}

cpu_locals_t* cpu_locals(uint32_t id) {
    if(!_cpuinit)
        return &_dummy_bsplocals;

    if(id > _cpumaxid)
        fatal("invalid cpu id: %u\n", id);

    return &_cpulcl[id];
}

uint32_t cpu_current_id() {
    if(!_cpuinit)
        return 0;

    return lapic_cpuid();
}
