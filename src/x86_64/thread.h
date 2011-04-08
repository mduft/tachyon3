/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include <thread.h>
#include <process.h>
#include "cpu.h"

struct _tag_thread_t {
    tid_t id;
    thread_state_t state;
    cpu_state_t cpu;
    process_t* parent;
    uintptr_t* stack;
};

