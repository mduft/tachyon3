/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "bmap.h"
#include "log.h"

#define BMAP_BITLOC(bit, idx, off) \
    asm volatile("div %4" : "=a"(idx), "=d"(off) : "d"(0), "a"(bit), "r"(sizeof(uintptr_t) * 8));

#define BMAP_NEXT_MUL(x, m)     ((((x) + (m - 1)) / m) * m)
#define BMAP_PREV_MUL(x, m)     (BMAP_NEXT_MUL(x, m) - m)

bitmap_t* bmap_new(size_t bitcnt) {
    /* TODO: memory allocation */
    return NULL;
}

uint8_t bmap_init(bitmap_t* handle_storage, void* storage, size_t bitcnt) {
    if(!handle_storage || !storage) {
        return FALSE;
    }

    handle_storage->storage = (uintptr_t*)storage;
    handle_storage->hint = 0;
    handle_storage->bits = bitcnt;
    handle_storage->allocated = 0;

    bmap_clear(handle_storage, 0);

    return TRUE;
}

void bmap_destroy(bitmap_t* bmap) {
    if(bmap && bmap->allocated) {
        /* TODO: free memory for storage and handle */
    }
}

uint8_t bmap_get(bitmap_t* bmap, size_t bit) {
    register size_t idx, off;

    /* Attention: may be false positive...! */
    if(bmap == NULL || bit > bmap->bits)
        return 0;

    BMAP_BITLOC(bit, idx, off);

    return ((bmap->storage[idx] >> off) & 1);
}

void bmap_set(bitmap_t* bmap, size_t bit, uint8_t value) {
    register size_t idx, off;

    if(bmap == NULL || bit > bmap->bits)
        return;

    BMAP_BITLOC(bit, idx, off);

    if(value) {
        bmap->storage[idx] |= (1 << off);
    } else {
        bmap->storage[idx] &= ~(1 << off);
    }
}

void bmap_clear(bitmap_t* bmap, uint8_t value) {
    if(bmap == NULL)
        return;

    bmap_fill(bmap, value, 0, bmap->bits - 1);
}

bool bmap_fill(bitmap_t* bmap, uint8_t value, size_t start, size_t end) {
    if(start >= bmap->bits || end >= bmap->bits || bmap == NULL)
        return FALSE;

    while(start < end) {
        /* TODO: optimize by setting blocks of size uintptr_t at once,
         *       if possible */
        bmap_set(bmap, start++, value);
    }

    return TRUE;
}

uint8_t bmap_search(bitmap_t* bmap, size_t* index, uint8_t value, size_t cnt, size_t mul, uint32_t flags) {
    register size_t start;
    register size_t con = 0;

    if(bmap == NULL || index == NULL || cnt == 0)
        return FALSE;

    /* sanitize values. */
    value = (value ? 1 : 0);
    if(mul == 0) mul = 1;

    if(flags & BMAP_SRCH_BACKWARD) {
        start = BMAP_PREV_MUL((flags & BMAP_SRCH_HINTED ? bmap->hint : bmap->bits - 1) - cnt, mul) + cnt;
        
        /* unsigned index, so if it reaches zero, it underflows */
        while(start <= bmap->bits) {
            if(bmap_get(bmap, start) == value) {
                ++con;

                if(con == cnt)
                    break;
            } else {
                con = 0;
                start = BMAP_PREV_MUL((start - 1) - cnt, mul) + cnt;
                continue;
            }
            --start;
        }

        /* start now is the beginning of the contigous bit block of
         * size cnt (at least). no need for further adjustment */
    } else {
        start = BMAP_NEXT_MUL((flags & BMAP_SRCH_HINTED ? bmap->hint : 0), mul);

        while(start < bmap->bits && con < cnt) {
            if(bmap_get(bmap, start) == value) {
                ++con;

                if(con == cnt)
                    break;
            } else {
                con = 0;
                start = BMAP_NEXT_MUL(start + 1, mul);
                continue;
            }
            ++start;
        }

        /* start now is one off the end of the contigous bit block. need
         * to adjust it, to be the beginning. */
        start -= (con - 1);
    }

    if((start % mul)) {
        fatal("internal implementation error!\n");
    }

    /* start underflows in backward searching when under zero,
     * so check for upper bounds only... */
    if(start >= bmap->bits) {
        if(flags & BMAP_SRCH_HINTED) {
            if(!bmap_search(bmap, index, value, cnt, mul, flags & ~(BMAP_SRCH_HINTED)))
                return FALSE;
        } else {
            return FALSE;
        }
    } else {
        *index = start;
    }

    bmap->hint = *index + cnt;
    return TRUE;
}

size_t bmap_fdeg(bitmap_t* bmap) {
    return ((bmap->set_cnt * 100) / bmap->bits);
}
