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
static uint64_t _ltsc = 0;
static uint64_t _tsc_per_increment = 0;

static rtc_calibrate_cb_t _calibrate = NULL;

static bool rtc_tick_handler(interrupt_t* state) {
    _systime += _increment;

    info("system time: %d ns\n", _systime);

    uint64_t cur_tsc = tsc_read();
    _tsc_per_increment = ((_tsc_per_increment + (cur_tsc - _ltsc)) / 2);
    _ltsc = cur_tsc;

    if(_calibrate) {
        if(!_calibrate(_systime, _increment))
            _calibrate = NULL;
    }

    return true;
}

static void rtc_init() {
    if(intr_state())
        fatal("RTC init must be called with interrupts still disabled!\n");

    intr_add(IRQ_NUM(8), rtc_tick_handler);

    rtc_set_rate(RTC_RATE_8HZ);

    outb(RTC_ADDR, RTC_REG_STATE_B);
    uint8_t b = inb(RTC_DATA);
    outb(RTC_ADDR, RTC_REG_STATE_B);
    outb(RTC_DATA, b | RTC_B_INT_PERIODIC);

    _rtc_bin = (b & RTC_B_BINARY);
    _rtc_24  = (b & RTC_B_24_HRS);

    info("realtime clock at %d ns per tick\n", _increment);

    ioapic_enable(8, lapic_cpuid());
}

INSTALL_EXTENSION(EXTP_KINIT, rtc_init, "realtime clock");

uint64_t rtc_systime() {
    return _systime;
}

uint8_t rtc_set_rate(uint8_t rate) {
    if(rate < RTC_RATE_8192HZ || rate > RTC_RATE_2HZ)
        fatal("RTC rate out of range: 0x%x!\n", rate);

    outb(RTC_ADDR, RTC_REG_STATE_A);
    uint8_t old = inb(RTC_DATA);
    outb(RTC_ADDR, RTC_REG_STATE_A);
    outb(RTC_DATA, RTC_A_DIVIDER_32768 | rate);

    _increment = RTC_INCREMENT_NS(rate);

    return old & 0xF;
}

void rtc_calibrate(rtc_calibrate_cb_t callback) {
    _calibrate = callback;
}
