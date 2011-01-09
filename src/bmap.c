/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "bmap.h"

#define BMAP_BITLOC(bit, idx, off) \
    asm volatile("div %4" : "=a"(idx), "=d"(off) : "d"(0), "a"(bit), "r"(sizeof(uintptr_t) * 8));

bitmap_t* bmap_new(size_t bitcnt) {
    /* TODO: memory allocation */
    return NULL;
}

uint8_t bmap_init(bitmap_t* handle_storage, void* storage, size_t bytes) {
    if(!handle_storage || !storage) {
        return FALSE;
    }

    handle_storage->storage = (uintptr_t*)storage;
    handle_storage->hint = 0;
    handle_storage->bits = (bytes * 8);
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
    BMAP_BITLOC(bit, idx, off);

    return ((bmap->storage[idx] >> off) & 1);
}

void bmap_set(bitmap_t* bmap, size_t bit, uint8_t value) {
    register size_t idx, off;
    BMAP_BITLOC(bit, idx, off);

    if(value) {
        bmap->storage[idx] |= (1 << off);
    } else {
        bmap->storage[idx] &= ~(1 << off);
    }
}

void bmap_clear(bitmap_t* bmap, uint8_t value) {
    bmap_fill(bmap, value, 0, bmap->bits - 1);
}

void bmap_fill(bitmap_t* bmap, uint8_t value, size_t start, size_t end) {
    while(start < end) {
        /* TODO: optimize by setting blocks of size uintptr_t at once,
         *       if possible */
        bmap_set(bmap, start++, value);
    }
}

uint8_t bmap_search(bitmap_t* bmap, size_t* index, uint8_t value, uint32_t flags) {
    size_t start;
    
    /* normalize value */
    value = (value ? 1 : 0);

    if(flags & BMAP_SRCH_BACKWARD) {
        start = (flags & BMAP_SRCH_HINTED ? bmap->hint - 1 : bmap->bits - 1);
        
        /* unsigned index, so if it reaches zero, it underflows */
        while(start <= bmap->bits) {
            if(bmap_get(bmap, start) == value) {
                break;
            }
            --start;
        }
    } else {
        start = (flags & BMAP_SRCH_HINTED ? bmap->hint + 1 : 0);

        while(start < bmap->bits) {
            if(bmap_get(bmap, start) == value) {
                break;
            }
            ++start;
        }
    }

    /* start underflows in backward searching when under zero,
     * so check for upper bounds only... */
    if(start >= bmap->bits) {
        if(flags & BMAP_SRCH_HINTED) {
            if(!bmap_search(bmap, index, value, flags & ~(BMAP_SRCH_HINTED)))
                return FALSE;
        } else {
            return FALSE;
        }
    } else {
        *index = start;
    }

    bmap->hint = *index;
    return TRUE;
}
