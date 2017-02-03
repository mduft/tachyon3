/* Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
 * This file is part of the 'tachyon' operating system. */

#include "tachyon.h"
#include "extp.h"
#include "log.h"
#include "tmr.h"
#include "list.h"
#include "kheap.h"
#include "systime.h"

// TODO: per CPU?
static tmr_gen_t* _generator = NULL;
static list_t* _timers = NULL;

typedef struct {
    uint64_t expire;    /**< timer expiration target time */
    uint64_t period;    /**< timer period, or zero if oneshot */
    tmr_cb_t callback;  /**< callback to call on timer expiration */
} tmr_t;

static void tmr_handle_tick();

static void tmr_init_handler(char const* tag, extp_func_t cb, char const* descr) {
    // TODO: choose best, for now, take first.
    if(_generator)
        return;

    tmr_gen_t* p = ((tmr_extp_t)cb)();

    if(!p || (p && !p->supported)) {
        trace("timer generator %s not available!\n", descr);
        return;
    }

        if(!p->init(tmr_handle_tick)) {
            warn("failed to initialize timer generator %s\n", descr);
            return;
        }

        info("chosen \"%s\" timer generator\n", descr);
        _generator = p;
    }

void tmr_init() {
    // find all timesource extensions, and choose one of them!
    // maybe make this configurable somehow? kernel command line?
    // config file?
    extp_iterate(EXTP_TIMERGEN, tmr_init_handler);

    if(!_generator)
        fatal("no timer generator found!\n");

    _timers = list_new();
}

static void tmr_resched_sorted(tmr_t* tmr) {
    list_node_t* preceeding = list_begin(_timers);

    while(preceeding) {
        tmr_t* other = (tmr_t*)preceeding->data;

        if(other->expire >= tmr->expire)
            break;

        preceeding = preceeding->next;
    }

    list_insert(_timers, preceeding, tmr);
}

static void tmr_handle_expired(tmr_t* tmr) {
    tmr->callback();

    list_remove(_timers, tmr);

    if(tmr->period) {
        uint64_t current = systime();
        tmr->expire = current + tmr->period;
        tmr_resched_sorted(tmr);
    } else {
        kheap_free(tmr);
    }
}

static void tmr_do_handle_tick() {
    list_node_t* node = list_begin(_timers);
    uint64_t current = systime();

    while(node) {
        tmr_t* tmr = (tmr_t*)node->data;
        node = node->next;

        if(tmr->expire <= current) {
            tmr_handle_expired(tmr);
        } else {
            break;
        }
    }
}

static void tmr_set_next_tick() {
    // first handle all that are due already to save reprogramming
    uint64_t current = systime();
    tmr_do_handle_tick();

    list_node_t* node = list_begin(_timers);

    if(node) {
        tmr_t* tmr = (tmr_t*)node->data;

        uint64_t timeout = tmr->expire - current;
        if(tmr->expire <= current + TMR_MIN_TIMEOUT) {
            timeout = TMR_MIN_TIMEOUT;
        }

        trace("scheduling timer for %lu (delta %lu)\n", tmr->expire, timeout);
        _generator->schedule(timeout);
    } else {
        _generator->schedule(TMR_MAX_TIMEOUT);
    }
}

bool tmr_schedule(tmr_cb_t callback, uint64_t us, bool oneshot) {
    tmr_t* pt = kheap_alloc(sizeof(tmr_t));

    if(!callback) {
        warn("timer without callback!\n");
        return false;
    }

    pt->expire = us + systime();
    pt->callback = callback;
    pt->period = (oneshot ? 0 : us);

    tmr_resched_sorted(pt);
    tmr_set_next_tick();

    return true;
}

static void tmr_handle_tick() {
    tmr_set_next_tick();
}

