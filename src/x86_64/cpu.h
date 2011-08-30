/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include <spl.h>
#include <thread.h>

/**
 * Describes the current state of the cpu associated with a thread's context
 */
typedef struct {
    uintptr_t rip;
    uintptr_t cs;
    uintptr_t rflags;
    uintptr_t rsp;
    uintptr_t ss;
    uintptr_t rbp;

    uintptr_t rax;
    uintptr_t rbx;
    uintptr_t rcx;
    uintptr_t rdx;

    uintptr_t rsi;
    uintptr_t rdi;

    uintptr_t r8;
    uintptr_t r9;
    uintptr_t r10;
    uintptr_t r11;
    uintptr_t r12;
    uintptr_t r13;
    uintptr_t r14;
    uintptr_t r15;

    uintptr_t cr0;
    uintptr_t cr2;
    uintptr_t cr3;
    uintptr_t cr4;
    uintptr_t cr8;
} x86_64_cpu_state_t;

struct _tag_thr_context_t;

typedef struct {
    struct _tag_thr_context_t* context;
} cpu_locals_t;

#define CPUID_0H                    0x00
#define CPUID_0H_MAX_CPUID(l)       (l.ax)

#define CPUID_01H                   0x01
#define CPUID_01H_V_STEPPING(l)     ((l.ax) & 0xF)
#define CPUID_01H_V_MODEL(l)        ((l.ax >> 4) & 0xF)
#define CPUID_01H_V_FAMILY(l)       ((l.ax >> 8) & 0xF)
#define CPUID_01H_V_EX_MODEL(l)     ((l.ax >> 16) & 0xF)
#define CPUID_01H_V_EX_FAMILY(l)    ((l.ax >> 20) & 0xFF)

#define CPUID_01H_BRAND_INDEX(l)    ((l.bx) & 0xFF)
#define CPUID_01H_CLFLUSH_LSZ(l)    (((l.bx >> 8) & 0xFF) * 8)
#define CPUID_01H_MAX_LOG_IDS(l)    ((l.bx >> 16) & 0xFF)
#define CPUID_01H_APIC_ID(l)        ((l.bx >> 24) & 0xFF)

#define CPUID_01H_SSE3(l)           ((l.cx) & (1))
#define CPUID_01H_PCLMULQDQ(l)      ((l.cx) & (1 << 1))
#define CPUID_01H_DTES64(l)         ((l.cx) & (1 << 2))
#define CPUID_01H_MONITOR(l)        ((l.cx) & (1 << 3))
#define CPUID_01H_DS_CPL(l)         ((l.cx) & (1 << 4))
#define CPUID_01H_VMX(l)            ((l.cx) & (1 << 5))
#define CPUID_01H_SMX(l)            ((l.cx) & (1 << 6))
#define CPUID_01H_EST(l)            ((l.cx) & (1 << 7))
#define CPUID_01H_TM2(l)            ((l.cx) & (1 << 8))
#define CPUID_01H_SSSE3(l)          ((l.cx) & (1 << 9))
#define CPUID_01H_CNXT_ID(l)        ((l.cx) & (1 << 10))
#define CPUID_01H_FMA(l)            ((l.cx) & (1 << 12))
#define CPUID_01H_CMPXCHG16B(l)     ((l.cx) & (1 << 13))
#define CPUID_01H_xTPR_UPD(l)       ((l.cx) & (1 << 14))
#define CPUID_01H_PDCM(l)           ((l.cx) & (1 << 15))
#define CPUID_01H_PCID(l)           ((l.cx) & (1 << 17))
#define CPUID_01H_DCA(l)            ((l.cx) & (1 << 18))
#define CPUID_01H_SSE41(l)          ((l.cx) & (1 << 19))
#define CPUID_01H_SSE42(l)          ((l.cx) & (1 << 20))
#define CPUID_01H_x2APIC(l)         ((l.cx) & (1 << 21))
#define CPUID_01H_MOVBE(l)          ((l.cx) & (1 << 22))
#define CPUID_01H_POPCNT(l)         ((l.cx) & (1 << 23))
#define CPUID_01H_TSC_DEADLINE(l)   ((l.cx) & (1 << 24))
#define CPUID_01H_AESNI(l)          ((l.cx) & (1 << 25))
#define CPUID_01H_XSAVE(l)          ((l.cx) & (1 << 26))
#define CPUID_01H_OSXSAVE(l)        ((l.cx) & (1 << 27))
#define CPUID_01H_AVX(l)            ((l.cx) & (1 << 28))

#define CPUID_01H_FPU(l)            ((l.dx) & (1))
#define CPUID_01H_VME(l)            ((l.dx) & (1 << 1))
#define CPUID_01H_DE(l)             ((l.dx) & (1 << 2))
#define CPUID_01H_PSE(l)            ((l.dx) & (1 << 3))
#define CPUID_01H_TSC(l)            ((l.dx) & (1 << 4))
#define CPUID_01H_MSR(l)            ((l.dx) & (1 << 5))
#define CPUID_01H_PAE(l)            ((l.dx) & (1 << 6))
#define CPUID_01H_MCE(l)            ((l.dx) & (1 << 7))
#define CPUID_01H_CMPXCHG8B(l)      ((l.dx) & (1 << 8))
#define CPUID_01H_LOCAL_APIC(l)     ((l.dx) & (1 << 9))
#define CPUID_01H_SEP(l)            ((l.dx) & (1 << 11))
#define CPUID_01H_MTRR(l)           ((l.dx) & (1 << 12))
#define CPUID_01H_PGE(l)            ((l.dx) & (1 << 13))
#define CPUID_01H_MCA(l)            ((l.dx) & (1 << 14))
#define CPUID_01H_CMOV(l)           ((l.dx) & (1 << 15))
#define CPUID_01H_PAT(l)            ((l.dx) & (1 << 16))
#define CPUID_01H_PSE_36(l)         ((l.dx) & (1 << 17))
#define CPUID_01H_PSN(l)            ((l.dx) & (1 << 18))
#define CPUID_01H_CLFLUSH(l)        ((l.dx) & (1 << 19))
#define CPUID_01H_DS(l)             ((l.dx) & (1 << 21))
#define CPUID_01H_ACPI(l)           ((l.dx) & (1 << 22))
#define CPUID_01H_MMX(l)            ((l.dx) & (1 << 23))
#define CPUID_01H_FXSR(l)           ((l.dx) & (1 << 24))
#define CPUID_01H_SSE(l)            ((l.dx) & (1 << 25))
#define CPUID_01H_SSE2(l)           ((l.dx) & (1 << 26))
#define CPUID_01H_SS(l)             ((l.dx) & (1 << 27))
#define CPUID_01H_HTT(l)            ((l.dx) & (1 << 28))
#define CPUID_01H_TM(l)             ((l.dx) & (1 << 29))
#define CPUID_01H_PBE(l)            ((l.dx) & (1 << 31))

#define CPUID_8_0H              0x80000000
#define CPUID_8_0H_MAX_EXTID(l) (l.ax)

#define CPUID_8_02H             0x80000002
#define CPUID_8_03H             0x80000003
#define CPUID_8_04H             0x80000004

/** Describes information returned from cpuid */
typedef struct {
    uint32_t ax;
    uint32_t bx;
    uint32_t cx;
    uint32_t dx;
} PACKED cpuid_leaf_t;

/**
 * Queries the cpus features
 *
 * @param leaf  the feature group to query
 * @return      a structure filled with information.
 */
cpuid_leaf_t cpuid(uint32_t leaf);

/**
 * Retrieves the locals for a cpu with the given ID.
 *
 * @param id    the id of the cpu to retrieve locals for.
 * @return      the cpus local variables.
 */
cpu_locals_t* cpu_locals(uint32_t id);

/**
 * Retrieves the ID of the current CPU.
 *
 * @return  the current cpus ID.
 */
uint32_t cpu_current_id();

