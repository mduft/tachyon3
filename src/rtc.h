/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include <tachyon.h>

#define RTC_RATE_8192HZ   (0x3)       /**< 122.0703 us */
#define RTC_RATE_4096HZ   (0x4)       /**< 244.1406 us */
#define RTC_RATE_2048HZ   (0x5)       /**< 488.2812 us */
#define RTC_RATE_1024HZ   (0x6)       /**< 976.5625 us - hardware default */
#define RTC_RATE_512HZ    (0x7)       /**<   1.9531 ms */
#define RTC_RATE_256HZ    (0x8)       /**<   3.9062 ms */
#define RTC_RATE_128HZ    (0x9)       /**<   7.8125 ms */
#define RTC_RATE_64HZ     (0xA)       /**<  15.6250 ms */
#define RTC_RATE_32HZ     (0xB)       /**<  31.2500 ms */
#define RTC_RATE_16HZ     (0xC)       /**<  62.5000 ms */
#define RTC_RATE_8HZ      (0xD)       /**< 125.0000 ms - tachyon default */
#define RTC_RATE_4HZ      (0xE)       /**< 250.0000 ms */
#define RTC_RATE_2HZ      (0xF)       /**< 500.0000 ms */

/** describes a callback for calibration using real time */
typedef bool (*rtc_calibrate_cb_t)(uint64_t systime, uint64_t increment);

/**
 * Returns the current system time in nanoseconds. The number
 * of ns is to be understood as offset from the time the system
 * timer started to tick.
 *
 * @return system uptime in nanoseconds
 */
uint64_t rtc_systime();

/**
 * Changes the rate at which the RTC fires. use the RTC_RATE_*
 * constants defined in this header.
 *
 * @param rate  the rate to fire interrupts at.
 * @return      the old rate.
 */
uint8_t rtc_set_rate(uint8_t rate);

/**
 * Retrieve the current increment per interrupt in nanoseconds
 *
 * @return number of ns incremented on each interrupt
 */
uint64_t rtc_get_increment();

/**
 * Temporarily switches the timer handler to call a specific
 * calibration callback for arbitrary calibration loops using
 * the realtime clock.
 * The rtc resets the calibration logic as soon as the callback
 * returns false.
 * 
 * @param cb    the calibration callback.
 */
void rtc_calibrate(rtc_calibrate_cb_t callback);
