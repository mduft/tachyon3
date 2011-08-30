/* copyright (c) 2011 by markus duft <mduft@gentoo.org>
 * this file is part of the 'tachyon' operating system. */

#include "cpu.h"
#include "thread.h"

thr_context_t* thr_ctx_get() {
    uint32_t id = cpu_current_id();
    return cpu_locals(id)->context;
}

void thr_ctx_set(thr_context_t* ctx) {
    cpu_locals(cpu_current_id())->context = ctx;
}

