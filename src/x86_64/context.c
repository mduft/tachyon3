/* copyright (c) 2011 by markus duft <mduft@gentoo.org>
 * this file is part of the 'tachyon' operating system. */

#include "cpu.h"
#include "thread.h"

thr_context_t* x86_64_ctx_get() {
    return cpu_locals(cpu_current_id())->context;
}

void x86_64_ctx_set(thr_context_t* ctx) {
    cpu_locals(cpu_current_id())->context = ctx;
}

