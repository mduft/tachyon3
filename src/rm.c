/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "rm.h"
#include "pmem.h"
#include "vmem.h"
#include "spc.h"
#include "extp.h"
#include "log.h"
#include "mem.h"
#include "x86/paging.h"

#include <contrib/rme2/rme.h>

/** convert rme state to our state */
#define RME2OURS(rme, ours) \
    (ours)->ax.dword = (rme)->AX.D; (ours)->bx.dword = (rme)->BX.D; \
    (ours)->cx.dword = (rme)->CX.D; (ours)->dx.dword = (rme)->DX.D; \
    (ours)->sp.dword = (rme)->SP.D; (ours)->bp.dword = (rme)->BP.D; \
    (ours)->si.dword = (rme)->SI.D; (ours)->di.dword = (rme)->DI.D; \
    (ours)->ss = (rme)->SS; (ours)->ds = (rme)->DS; \
    (ours)->es = (rme)->ES; (ours)->cs = (rme)->CS; \
    (ours)->ip = (rme)->IP; (ours)->fl = (rme)->Flags;

/** convert our state to rme state */
#define OURS2RME(ours, rme) \
    (rme)->AX.D = (ours)->ax.dword; (rme)->BX.D = (ours)->bx.dword; \
    (rme)->CX.D = (ours)->cx.dword; (rme)->DX.D = (ours)->dx.dword; \
    (rme)->SP.D = (ours)->sp.dword; (rme)->BP.D = (ours)->bp.dword; \
    (rme)->SI.D = (ours)->si.dword; (rme)->DI.D = (ours)->di.dword; \
    (rme)->SS = (ours)->ss; (rme)->DS = (ours)->ds; \
    (rme)->ES = (ours)->es; (rme)->CS = (ours)->cs; \
    (rme)->IP = (ours)->ip; (rme)->Flags = (ours)->fl;

/** log RME errors */
#define SWITCH_LOGERR           \
    case RME_ERR_INVAL:         \
        warn("rm: invalid parameters\n");   \
    case RME_ERR_BADMEM:        \
        warn("rm: bad memory access\n");    \
    case RME_ERR_UNDEFOPCODE:   \
        warn("rm: undefined opcode\n");     \
    case RME_ERR_DIVERR:        \
        warn("rm: division fault\n");       \
    default:                    \
        warn("rm: unknown error\n");        \

/** emit RME normal call */
#define RME_FUNC_Call       RME_Call(&rme_state)

/** emit RME interrupt call */
#define RME_FUNC_CallInt    RME_CallInt(&rme_state, vec)

/** 
 * call RME with the given function. this makes the actual
 * function calling the macro return.
 *
 * @param func  either "Call" or "CallInt" (without quotes).
 * @param state state in our format.
 * @param vec   interrupt to call if func is CallInt.
 */
#define RME_DO(func, state, vec) \
    tRME_State rme_state;                       \
    memset(&rme_state, 0, sizeof(rme_state));   \
    OURS2RME(state, &rme_state);                \
    rme_state.Memory[0] = (void*)RM_VIRTUAL;    \
    switch(RME_FUNC_##func) {                   \
    case RME_ERR_OK:                            \
        RME2OURS(&rme_state, state);            \
        return true;                            \
    SWITCH_LOGERR                               \
    }                                           \
    return false;                               \


void rm_init() {
    register size_t mem = RME_BLOCK_SIZE;
    register uintptr_t vmem = RM_VIRTUAL + RME_BLOCK_SIZE;

    /* map the virtual memory region resreved for real mode to physical NULL.
     * Be aware that the physical memory used by real mode is _NOT_ reserved.
     * Any call to pmem_alloc() may allocate memory within this, and use it.
     * This needs to be taken into account when using real mode memory. */
    while(mem) {
        mem -= PAGE_SIZE_4K;
        vmem -= PAGE_SIZE_4K;

        vmem_map(spc_current(), mem, (void*)vmem, PG_WRITABLE);
    }

    info("real-mode emulation initialized.\n");
}

bool rm_int(uint8_t vec, rm_state_t* state) {
    RME_DO(CallInt, state, vec);
}

bool rm_call(rm_state_t* state) {
    RME_DO(Call, state, 0);
}
