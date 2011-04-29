/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include <spl.h>
#include <thread.h>

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
} x86_64_cpu_state_t;


