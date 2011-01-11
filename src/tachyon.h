/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#ifdef __X86__
# include "x86/types.h"
# include "x86/defs.h"
#elif defined(__X86_64__)
# include "x86_64/types.h"
# include "x86_64/defs.h"
#else
# error "unknown architecture!"
#endif

#define NULL    (void*)0
#define TRUE    1
#define FALSE   0

#define ALIGN_DN(x, a)  ((x) & ~((a) - 1))
#define ALIGN_UP(x, a)  ALIGN_DN(x + a, a)

typedef __builtin_va_list va_list;
#define va_start(v, l)      __builtin_va_start(v,l)
#define va_end(v)           __builtin_va_end(v)
#define va_arg(v, l)        __builtin_va_arg(v,l)

#define PACKED              __attribute__((__packed__))
#define SECTION(x)          __attribute__((section(x)))
#define UNUSED              __attribute__((unused))
#define NORETURN            __attribute__((noreturn))

/* 
 * .----------------------------------.
 * | common types for x86 and x86_64  |
 * '----------------------------------' 
 */

typedef signed      char        int8_t;
typedef signed      short       int16_t;
typedef signed      int         int32_t;
typedef signed      long long   int64_t;
typedef intptr_t                ssize_t;

typedef unsigned    char        uint8_t;
typedef unsigned    short       uint16_t;
typedef unsigned    int         uint32_t;
typedef unsigned    long long   uint64_t;
typedef uintptr_t               size_t;

typedef intptr_t    intmax_t;
typedef uintptr_t   uintmax_t;
typedef intmax_t    off_t;

typedef uintmax_t   pid_t;
typedef pid_t       tid_t;

typedef uintptr_t   phys_addr_t;
typedef phys_addr_t aspace_t;

typedef uint8_t     bool;

/**
 * describes the available information when an interrupt happens.
 */
typedef struct {
    uintptr_t   num;    /**< the interrupt number */
    uintptr_t   code;   /**< the error code, or zero if none */
    uintptr_t   ip;     /**< the interrupted location */
    uintptr_t   cs;     /**< the code segment of the interrupted location. */
    uintptr_t   flags;  /**< the eflags/rflags of the interrupted thread. */

    uintptr_t   sp;     /**< (only on ring change). the original stack pointer */
    uintptr_t   ss;     /**< (only on ring change). the original stack segment */
} interrupt_t;

/* 
 * .----------------------------------.
 * | common basic functions           |
 * '----------------------------------' 
 */

/**
 * Aborts execution of the kernel, and shows a dump of some CPU
 * state information, which can be helpfull. This function never
 * returns to the caller.
 */
void abort() NORETURN;

