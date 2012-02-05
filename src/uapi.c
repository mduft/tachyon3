/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "uapi.h"
#include "vmem.h"
#include "ldsym.h"
#include "paging.h"

static uapi_desc_t const SECTION(SECT_USER_DATA) uapi_desc = {
    .syscall=uapi_sysc_call
};

uintptr_t SECTION(SECT_USER_CODE) uapi_sysc_call(syscall_t call, uintptr_t p0, uintptr_t p1) {
    uintptr_t res;

    // TODO: sysenter, etc.?
    
    asm volatile(
        "\tint %1\n"
        "\tmov %%rax, %0\n"
        : "=a"(res) 
        : "i"(SYSC_INTERRUPT), "D"(call), "S"(p0), "d"(p1));

    return res;
}

void SECTION(SECT_USER_CODE) uapi_thr_trampoline(thread_t* thread, thread_start_t entry) {
    entry(&uapi_desc);

    thread->state = Exited;
    sysc_call(SysSchedule, 0, 0);

    /* never reached - as the thread is aborting, it will never be re-scheduled */
}

static bool _uapi_map_from_to(spc_t space, uintptr_t* sv, uintptr_t* sp, uintptr_t* ev, uintptr_t* ep) {
    // attention: keep sv and sp synchronous!
    while(sv < ev && sp < ep) {
        if(!vmem_map(space, (uintptr_t)sp, sv, PG_USER | PG_GLOBAL))
            return false;
        
        sv += PAGE_SIZE_4K;
        sp += PAGE_SIZE_4K;
    }

    return true;
}

/* NOT mapped to user space! */
bool uapi_map(spc_t space) {
    // TODO: unmap in case of error!

    if(!_uapi_map_from_to(space, &_core_vma_user_code, &_core_lma_user_code, &_core_vma_user_ecode, &_core_lma_user_ecode))
        return false;

    if(!_uapi_map_from_to(space, &_core_vma_user_data, &_core_lma_user_data, &_core_vma_user_edata, &_core_lma_user_edata))
        return false;

    return true;
}

///// TEST /////

void SECTION(SECT_USER_CODE) test_thr2(uapi_desc_t const* uapi) {
    /// HELP i cannot output anything here!
    return;
}
