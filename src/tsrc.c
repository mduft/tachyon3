/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "tachyon.h"
#include "extp.h"
#include "log.h"
#include "tsrc.h"
#include "list.h"
#include "kheap.h"

// TODO: per CPU?
static tsrc_t* _tsrc = NULL;
static list_t* _timers = NULL;

typedef struct {
    millis_t expire;    /**< timer expiration target time */
    millis_t period;    /**< timer period, or zero if oneshot */
    tsrc_cb_t callback; /**< callback to call on timer expiration */
} tmr_t;

static void tsrc_handle_tick();

static void tsrc_init_handler(char const* tag, extp_func_t cb, char const* descr) {
    // TODO: choose best, for now, take first.
    if(_tsrc)
        return;

    tsrc_t* p = ((tsrc_extp_t)cb)();

    if(!p || (p && !p->supported)) {
        trace("timesource %s not available!\n", descr);
        return;
    }

    if(!p->init(tsrc_handle_tick)) {
        warn("failed to initialize timesource %s\n", descr);
        return;
    }

    info("chosen %s time source\n");
    _tsrc = p;
}

static void tsrc_init() {
    // find all timesource extensions, and choose one of them!
    // maybe make this configurable somehow? kernel command line?
    // config file?
    extp_iterate(EXTP_TIMESOURCE, tsrc_init_handler);

    if(!_tsrc)
        fatal("no timesource found!\n");

    _timers = list_new();
}

INSTALL_EXTENSION(EXTP_KINIT, tsrc_init, "time source");

static void tsrc_resched_sorted(tmr_t* tmr) {
    list_node_t* preceeding = list_begin(_timers);

    while(preceeding) {
        tmr_t* other = (tmr_t*)preceeding->data;

        if(other->expire >= tmr->expire)
            break;

        preceeding = preceeding->next;
    }

    list_insert(_timers, preceeding, tmr);
}

static void tsrc_handle_expired(tmr_t* tmr) {
    tmr->callback();

    list_remove(_timers, tmr);

    if(tmr->period) {
        tmr->expire += tmr->period;
        tsrc_resched_sorted(tmr);
    } else {
        kheap_free(tmr);
    }
}

static void tsrc_do_handle_tick() {
    list_node_t* node = list_begin(_timers);
    millis_t current = _tsrc->current_ticks();

    while(node) {
        tmr_t* tmr = (tmr_t*)node->data;

        if(tmr->expire <= current) {
            tsrc_handle_expired(tmr);
        } else {
            break;
        }

        // beware of in-loop-modification (expired timers are removed)
        node = list_begin(_timers);
    }
}

static void tsrc_set_next_tick() {
    // first handle all that are due already to save reprogramming
    tsrc_do_handle_tick();

    list_node_t* node = list_begin(_timers);

    if(node) {
        tmr_t* tmr = (tmr_t*)node->data;
        _tsrc->schedule(tmr->expire);
    } else {
        _tsrc->schedule(_tsrc->current_ticks() + TSRC_MAX_TICK);
    }
}

bool tsrc_schedule(tsrc_cb_t callback, millis_t ms, bool oneshot) {
    tmr_t* pt = kheap_alloc(sizeof(tmr_t));

    if(!callback) {
        warn("timer without callback!\n");
        return false;
    }

    pt->expire = _tsrc->current_ticks() + ms;
    pt->callback = callback;
    pt->period = (oneshot ? ms : 0);

    tsrc_resched_sorted(pt);
    tsrc_set_next_tick();

    return true;
}

static void tsrc_handle_tick() {
    tsrc_set_next_tick();
}

