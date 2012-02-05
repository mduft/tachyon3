/* copyright (c) 2011 by markus duft <mduft@gentoo.org>
 * this file is part of the 'tachyon' operating system. */

#include "tss.h"
#include "dyngdt.h"

#include <kheap.h>
#include <mem.h>
#include <stka.h>
#include <spc.h>
#include <log.h>

#include "gdt.h"
#include "paging.h"
#include "vmem.h"
#include "vmem_mgmt.h"

static stack_allocator_t* kstack_allocator = NULL;

static void _tss_publish_mapping(stack_t* stack, stack_allocator_t* allocator) {
    uintptr_t start_v = stack->mapped;
    
    while(start_v < stack->top) {
        // remember mappings for each page.
        vmem_mgmt_add_global_mapping(vmem_resolve(spc_current(), 
            (void*)start_v), (void*)start_v, allocator->desc.pg_fl);
        
        start_v += STK_PAGESIZE;
    }
}

void tss_init() {
    if(kstack_allocator == NULL) {
        stack_allocator_desc_t desc = {
            .top = KSHEAP_END,
            .bottom = KSHEAP_START,
            .space = spc_current(),
            .pg_fl = (PG_WRITABLE),
            .fixed = true
        };

        kstack_allocator = stka_new(&desc);

        if(!kstack_allocator)
            fatal("failed to create kernel stack allocator\n");
    }

    tss_t* tss = kheap_alloc(sizeof(tss_t));
    memset(tss, 0, sizeof(tss_t));

    /* allocate IST stacks, ring stacks...
     * the stack descriptor is currently discarded, as we never want/need
     * to free those stacks again... maybe in the future? cpu hotplug? */
    stack_t* fstk = stka_alloc(kstack_allocator);
    tss->ist[IST_FAULT_STACK] = fstk->top;
    info("IST fault stack at %p - %p\n", fstk->mapped, fstk->top);

    stack_t* sstk = stka_alloc(kstack_allocator);
    tss->ist[IST_SYSC_STACK] = sstk->top;
    info("IST system call stack at %p - %p\n", sstk->mapped, sstk->top);

    stack_t* lstk = stka_alloc(kstack_allocator);
    tss->ist[IST_LLHW_STACK] = lstk->top;
    info("IST low-level stack at %p - %p\n", lstk->mapped, lstk->top);

    info("publishing global stack mappings");
    _tss_publish_mapping(fstk, kstack_allocator);
    _tss_publish_mapping(sstk, kstack_allocator);
    _tss_publish_mapping(lstk, kstack_allocator);

    dyngdt_set(GDT_KTSS, (uintptr_t)tss, sizeof(tss_t), GDT_TYPE_TSS, 0, true, true);
}
