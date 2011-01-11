/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "log.h"
#include "extp.h"
#include "string.h"

#define MAX_LOG_DESTINATIONS     32
#define MAX_LOG_WRITE_BUFFER     4096
#define LOG_DEFAULT_LEVEL        Warning

/**
 * The actual type for the extension point callback.
 *
 * @param msg   the formatted message to write.
 */
typedef void (*log_writer_t)(char const* msg);

/**
 * The structure holding each log destination. This holds the
 * writer itself and associated information.
 *
 * Note: the name field is derived from the extension point
 *       description field. The text is freely assignable, and
 *       may appear more than once (CGA uses "screen" for example).
 */
typedef struct {
    log_writer_t writer;    /**< the target function to write a message */
    log_level_t  level;     /**< the maximum acceptable level for this writer */
    char const*  name;      /**< the name for this destination. */
} log_destination_t;

/**
 * Holds all log destinations. Initially, none of the destinations
 * is set, but log_init() gathers all the extension points, and
 * fills this array
 */
static log_destination_t destinations[MAX_LOG_DESTINATIONS];

/**
 * This is a callback for the extp_iterate() function, which is used
 * to iterate all log writer extensions. So this is called once for
 * each log writer extension available.
 *
 * @param tag       must be EXPT_LOG_WRITER in this case.
 * @param writer    the extension callback, must be of actual type log_writer_t.
 * @param descr     the extension description, used as name for the writer.
 */
static void log_add_writer(char const* tag, extp_func_t writer, char const* descr) {
    register size_t idx;

    for(idx = 0; idx < MAX_LOG_DESTINATIONS; ++idx) {
        if(!destinations[idx].writer) {
            destinations[idx].writer = (log_writer_t)writer;
            destinations[idx].level  = LOG_DEFAULT_LEVEL;
            destinations[idx].name   = descr;
            return;
        }
    }

    warn("maximum writer count exceeded, cannot add %p\n", writer);
}

/**
 * This function is used to format an unsigned number with the
 * specified base into a string buffer.
 *
 * @param buf       the buffer to write the formatted number to.
 * @param number    the number to format.
 * @param base      the base to use.
 * @param fill      the fill character, of width > actual length.
 * @param width     the field width. the buffer is filled with 
 *                  fill characters, if the number is not this long.
 */
static void log_format_u(char* buf, uintmax_t number, uint8_t base, char fill, size_t width) {
    register char* p = buf;

    do {
        register uintmax_t rem = number % base;
        *p++ = (rem < 10) ? rem + '0' : rem + 'a' - 10;
    } while(number /= base);

    if(fill) {
        while((size_t)(p - buf) < width) *p++ = fill;
    }

    *p = 0;

    {
        /* reverse the buffer */
        register char* p1 = buf;
        register char* p2 = p - 1;

        while(p1 < p2) {
            register char tmp = *p1;
            *p1++ = *p2; 
            *p2-- = tmp;
        }
    }
}

/**
 * This function is used to format a signed number with the
 * specified base into a string buffer. It calles log_format_u
 * to do so.
 *
 * @param buf       the buffer to write the formatted number to.
 * @param number    the number to format.
 * @param base      the base to use.
 * @param fill      the fill character, of width > actual length.
 * @param width     the field width. the buffer is filled with 
 *                  fill characters, if the number is not this long.
 */
static void log_format_s(char* buf, intmax_t number, uint8_t base, char fill, size_t width) {
    if(number < 0) {
        buf[0] = '-';
        log_format_u(&buf[1], (uintmax_t)(-number), base, fill, width);
    } else {
        log_format_u(buf, (uintmax_t)(number), base, fill, width);
    }
}

/**
 * Parses a string, and extracts a number from it, stopping
 * extraction at the first non-number character.
 *
 * @param str   the string to parse.
 * @return      the extracted number.
 */
static uintmax_t log_parse_number(char const* str) {
    uintmax_t val = 0;

    while(*str >= '0' && *str <= '9') {
        val*= 10;
        val += *str - '0';
        ++str;
    }

    return val;
}

/**
 * Formats a log message into a specified buffer.
 *
 * @param buf   the buffer to write to.
 * @param len   the length of the buffer. if the message grows longer
 *              it is cut off.
 * @param fmt   the format string.
 * @param args  the variable argument list used for the format parameters.
 */
static void log_format_message(char* buf, size_t len, char const* fmt, va_list args) {
    char c;
    char* p = buf;

    #define CHECKED_APPEND(c) { if((size_t)(p - buf) < len) *p++ = (c); }

    while((c = *fmt++) != 0) {
        if(c != '%') {
            CHECKED_APPEND(c);
        } else {
            char  temp[32];
            char * ptemp = temp;
            char const* append = 0;
            bool lng = FALSE;
            uint32_t width = 0;
            c = *fmt++;

            /* field width specification */
            if(c >= '0' && c <= '9') {
                width = log_parse_number((fmt - 1));

                while(c >= '0' && c <= '9') {
                    c = *fmt++;
                }
            }

            /* "long" switch */
            if(c == 'l') {
                lng = TRUE;
                c = *fmt++;
            }

            #define DO_NUMBER(s, t, b, f, w)                \
                log_format_##s(ptemp, va_arg(args, t), b, f, w); \
                append = temp;                              \
                goto string;

            switch(c) {
            case 'd':   if(lng) { DO_NUMBER(s, int64_t, 10, ' ', width);   }
                        else    { DO_NUMBER(s, int32_t, 10, ' ', width);  }
            case 'u':   if(lng) { DO_NUMBER(s, uint64_t, 10, ' ', width);  }
                        else    { DO_NUMBER(s, uint32_t, 10, ' ', width); }
            case 'x':   if(lng) { DO_NUMBER(u, uint64_t, 16, '0', width);  }
                        else    { DO_NUMBER(u, uint32_t, 16, '0', width); }
            case 'p':   *ptemp++ = '0'; *ptemp++ = 'x'; DO_NUMBER(u, uintmax_t, 16, '0', sizeof(uintmax_t) * 2);
            case 'c':
                temp[0] = (char)(va_arg(args, int));
                temp[1] = 0;
                append = temp;
                goto string;
            case 's':
                append = va_arg(args, char const*);
                /* fall through to label */
            string:
                if(!append)
                    append = "(null)";

                {
                    size_t len = 0;
                    while(*append) {
                        CHECKED_APPEND(*append++);
                        len++;
                    }

                    while(len++ < width) {
                        CHECKED_APPEND(' ');
                    }
                }

                break;

            default:
                append = "(unknown format)";
                goto string;
            }
        }

        if((size_t)(p - buf) >= len) break;
    }

    /* terminate */
    *p = 0;

    #undef DO_NUMBER
    #undef CHECKED_APPEND

}

void log_init() {
    extp_iterate(EXTP_LOG_WRITER, log_add_writer);
}

void log_set_level(char const* dest, log_level_t lvl) {
    register size_t idx;

    for(idx = 0; idx < MAX_LOG_DESTINATIONS; ++idx) {
        if((destinations[idx].writer) &&
                (!dest || strcmp(dest, destinations[idx].name) == 0)) {
            destinations[idx].level = lvl;
        }
    }
}

void log_write(log_level_t lvl, char const* fmt, ...) {
    /* TODO: lock this ... */
    register size_t idx;
    char buf[MAX_LOG_WRITE_BUFFER];
    va_list lst;

    va_start(lst, fmt);
    log_format_message(buf, sizeof(buf), fmt, lst);
    va_end(lst);

    for(idx = 0; idx < MAX_LOG_DESTINATIONS; ++idx) {
        if(destinations[idx].writer && destinations[idx].level >= lvl) {
            destinations[idx].writer(buf);
        }
    }
}

