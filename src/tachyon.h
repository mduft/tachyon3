/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
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
#define true    1
#define false   0

/** stringize the given argument */
#define stringize(x) #x

/** resolve a preprocessor macro and stringize the result */
#define rstringize(x) stringize(x)

#define ALIGN_DN(x, a)  ((x) & ~((a) - 1))
#define ALIGN_UP(x, a)  ALIGN_DN(x + (a - 1), a)
#define ALIGN_RST(x, a) ((x) & ((a) - 1))

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
typedef phys_addr_t spc_t;

typedef uint8_t     bool;

typedef uint32_t    cpuid_t;

/**
 * Describes the initial state when tachyon takes control. At the
 * moment this stores the multiboot informations, if available.
 */
typedef struct {
    uint32_t ax;    /**< value of the eax register passed by the loader */
    uint32_t bx;    /**< value of the ebx register passed by the loader */
} init_state_t;

/**
 * The initial boot state. Accessible to all.
 */
extern init_state_t const boot_state;

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

