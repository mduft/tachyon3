/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "rtc.h"
#include "tsc.h"
#include "ioapic.h"
#include "lapic.h"

#include <extp.h>
#include <intr.h>
#include <io.h>
#include <log.h>
#include <intr.h>
#include <systime.h>

#define RTC_ADDR    0x70
#define RTC_DATA    0x71

#define RTC_REG_SECONDS     0x0
#define RTC_REG_ALARM_SEC   0x1
#define RTC_REG_MINUTES     0x2
#define RTC_REG_ALARM_MIN   0x3
#define RTC_REG_HOURS       0x4
#define RTC_REG_ALARM_HRS   0x5
#define RTC_REG_WEEKDAY     0x6
#define RTC_REG_DAYOFMON    0x7
#define RTC_REG_MONTH       0x8
#define RTC_REG_YEAR        0x9

#define RTC_REG_STATE_A     0xA
#define RTC_REG_STATE_B     0xB
#define RTC_REG_STATE_C     0xC

#define RTC_A_DIVIDER_32768 (2 << 4)    /**< default divider, only stable time source */
#define RTC_A_UPD_PROGRESS  (1 << 7)    /**< time update in progress */

#define RTC_B_AUTO_DST      (1)         /**< enables automatic DST switching in April & October */
#define RTC_B_24_HRS        (1 << 1)    /**< 24 hours mode, else 12 hours */
#define RTC_B_BINARY        (1 << 2)    /**< Binary clock, BCD otherwise. */
#define RTC_B_SQ_WAVE       (1 << 3)    /**< Square wave output mode */
#define RTC_B_INT_UPD_END   (1 << 4)    /**< send interrupt when clock updated */
#define RTC_B_INT_ALARM     (1 << 5)    /**< interrupt when alarm time expired */
#define RTC_B_INT_PERIODIC  (1 << 6)    /**< enable periodic interrupt at programmed rate */
#define RTC_B_CLOCK_FREEZE  (1 << 7)    /**< freeze clock to enable setting it */

#define RTC_INCREMENT_NS(x) \
    ((1*1000*1000*1000) / (32768 >> ((x)-1)))

static uint64_t _systime = 0;
static bool _rtc_bin;
static bool _rtc_24;
static uint32_t _increment;
static uint64_t _tsc;
static uint64_t _tsc_rate;

static rtc_calibrate_cb_t _calibrate = NULL;

static bool rtc_tick_handler(interrupt_t* state) {
    intr_disable();
    _systime += _increment;

    uint64_t _ltsc = _tsc;
    _tsc = tsc_read();

    /* average tsc rate per clock tick */
    _tsc_rate = (_tsc_rate + (_tsc - _ltsc)) / 2;

    if(_calibrate) {
        if(!_calibrate(_systime, _increment))
            _calibrate = NULL;
    }

    // need to read/discard register C, which gives information about the type
    // of interrupt (which only can be periodic for now), to get another
    // one from the chip. otherwise, IRQ8 will be inhibited.
    outb(RTC_REG_STATE_C, RTC_ADDR);
    (void)inb(RTC_DATA);

    lapic_eoi();
    intr_enable();

    return true;
}

static void rtc_init() {
    intr_add(IRQ_NUM(8), rtc_tick_handler);

    rtc_set_rate(RTC_RATE_32HZ);

    outb(RTC_REG_STATE_B, RTC_ADDR);
    uint8_t b = inb(RTC_DATA);
    outb(RTC_REG_STATE_B, RTC_ADDR);
    outb(b | RTC_B_INT_PERIODIC, RTC_DATA);

    _rtc_bin = (b & RTC_B_BINARY);
    _rtc_24  = (b & RTC_B_24_HRS);

    info("realtime clock at %d ns per tick, bin: %d, 24h: %d\n", _increment, _rtc_bin, _rtc_24);

    ioapic_enable(8, lapic_cpuid());
}

static systime_desc_t const* rtc_ext() {
    static systime_desc_t _desc = {
        .systime_init_func = rtc_init,
        .systime_ns_func = rtc_systime,
        .accuracy = Good
    };

    return &_desc;
}

INSTALL_EXTENSION(EXTP_SYSTIME, rtc_ext, "realtime clock");

uint64_t rtc_systime() {
    uint64_t _nsoff = (_tsc_rate / _increment) * (tsc_read() - _tsc);

    trace("tsc would report %ld ns offset: rate: %ld, incr: %ld, tsc: %ld, now: %ld\n", _nsoff, _tsc_rate, _increment, _tsc, tsc_read());

    if(_nsoff > _increment) {
        debug("uh oh - TSC running too fast!\n");
        _nsoff = 0;
    }

    return _systime + _nsoff;
}

uint8_t rtc_set_rate(uint8_t rate) {
    if(rate < RTC_RATE_8192HZ || rate > RTC_RATE_2HZ)
        fatal("RTC rate out of range: 0x%x!\n", rate);

    outb(RTC_REG_STATE_A, RTC_ADDR);
    uint8_t old = inb(RTC_DATA);
    outb(RTC_REG_STATE_A, RTC_ADDR);
    outb(RTC_A_DIVIDER_32768 | rate, RTC_DATA);

    _increment = RTC_INCREMENT_NS(rate);

    return old & 0xF;
}

void rtc_calibrate(rtc_calibrate_cb_t callback) {
    _calibrate = callback;
}
