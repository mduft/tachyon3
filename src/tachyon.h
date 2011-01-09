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
typedef phys_addr_t vspace_t;

typedef uint8_t     bool;

typedef struct {
    uintptr_t   num;
    uintptr_t   code;
    uintptr_t   ip;
    uintptr_t   cs;
    uintptr_t   flags;

    /* on x86 only on ring change */
    uintptr_t   sp;
    uintptr_t   ss;
} interrupt_t;

