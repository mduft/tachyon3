/* copyright (c) 2011 by markus duft <mduft@gentoo.org>
 * this file is part of the 'tachyon' operating system. */

#pragma once

#include <tachyon.h>

/**
 * Describes a selector in the dynamic GDT.
 */
typedef struct {
    union {
        struct {
            uint16_t limit_low;         /**< bits  1..16 of limit */
            uint16_t base_low;          /**< bits  1..16 of base */
            uint32_t base_m_low : 8;    /**< bits 17..24 of base */
            uint32_t type       : 4;
            uint32_t seg_sel    : 1;    /**< system descriptor (tss, ...) (0), or code/data seg (1) */
            uint32_t dpl        : 2;
            uint32_t present    : 1;
            uint32_t limit_high : 4;
            uint32_t _avl0      : 1;
            uint32_t large      : 1;    /**< 64 bit code segment, 0 for tss. */
            uint32_t opsz       : 1;    /**< default operand size (0=16bit, 1=32bit), 0 for tss */
            uint32_t granularity: 1;    /**< if set, the limit is in 4KB units, otherwise bytes */
            uint32_t base_m_high: 8;    /**< bits 25..32 of base */
        } fields;
        uint64_t bytes;
    };
} PACKED selector_t;

/**
 * Describes the second part of a 64 bit descriptor, as this
 * requires a larger base address field, which is written
 * into a second descriptor slot.
 */
typedef struct {
    union {
        struct {
            uint32_t base_high;         /**< additional high address bits 33..64 of base */
            uint32_t _res0      : 8;
            uint32_t _zero3     : 5;
            uint32_t _res1      : 19;
        } fields;
        uint64_t bytes;
    };
} PACKED selector_ext_t;

/**
 * Creates a new dyanmic GDT for the current cpu.
 *
 * @attention calling this function globally locks the dynamic gdt lock.
 *            a matching call to dyngdt_activate_and_unlock() is required!
 */
void dyngdt_init_and_lock();

/**
 * Sets a gate or system descriptor in the given dynamic GDT.
 *
 * @param sel   the selector to set.
 * @param base  the base address to write.
 * @param limit the limit for this segment.
 * @param flags the flags used to create the descriptor.
 * @param dpl   the desired descriptor protection level (0..2).
 * @param large whether the selector is supposed to target 32 or 64 bit.
 * @param sys   indicates whether this should be a system or a code/data selector.
 */
void dyngdt_set(uint16_t sel, uintptr_t base, uint32_t limit, uint32_t flags, uint8_t dpl, bool large, bool sys);

/**
 * Makes the dynamic GDT active, removing the startup GDT.
 */
void dyngdt_activate_and_unlock();
