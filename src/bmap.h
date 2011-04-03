/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

#define BMAP_SRCH_FORWARD   0x0
#define BMAP_SRCH_BACKWARD  0x1
#define BMAP_SRCH_HINTED    0x2 /**< use and update the hint field during the search */

/** The bitmap header. */
typedef struct {
    uintptr_t* storage; /**< The storage used by the bitmap. */
    size_t bits;        /**< The number of bits in the bmap */
    size_t hint;        /**< A hint for bmap_next_bit() */
    bool allocated;     /**< Whether this is allocated or init()'ed */
    size_t set_cnt;        /**< The number of set (as opposed to unset) bits. */
} bitmap_t;

/** 
 * Allocates a new BitMap.
 *
 * @param bitcnt    the count of bits that should be addressable.
 * @return          the pointer to the new bitmap.
 */
bitmap_t* bmap_new(size_t bitcnt);

/**
 * Initializes the given memory reagions to contain a bitmap.
 * This is especially usefull, as long as memory management is
 * not yet fully initialized and static storage locations are
 * to be used.
 *
 * @param handle_storage    memory location of size sizeof(bitmap_t)
 * @param storage           memory location of size (bitcnt/sizeof(uintptr_t))
 * @param bitcnt            the number of bits in the bitmap (storage needs
 *                          to be large enough to contain this number of bits!)
 * @return                  true on success, false on failure.
 */
bool bmap_init(bitmap_t* handle_storage, void* storage, size_t bitcnt);

/**
 * Deallocates a previously allocated bitmap. Does nothing,
 * in case the bitmap was not allocated but initialized with
 * bmap_init() only.
 *
 * @param bmap  the bitmap to destruct.
 */
void bmap_delete(bitmap_t* bmap);

/**
 * Returns the value of the given bit in the bitmap.
 *
 * @param bmap  the bitmap to use.
 * @param bit   the index of the bit to return.
 * @return      the value of the given bit.
 */
uint8_t bmap_get(bitmap_t* bmap, size_t bit);

/**
 * Sets the given bit to the given value.
 *
 * @param bmap  the bitmap to use.
 * @param bit   the index of the bit to set.
 * @param value the value (0 or 1 (actually !0)) to set.
 */
void bmap_set(bitmap_t* bmap, size_t bit, uint8_t value);

/**
 * Clears all bits of a bitmap to a given value.
 *
 * @param bmap  the bitmap to use.
 * @param value the target value for all bits in the bitmap
 */
void bmap_clear(bitmap_t* bmap, uint8_t value);

/**
 * Sets all bits withing a given region in a bitmap.
 *
 * @param bmap  the bitmap to use.
 * @param value the value to set each bit to.
 * @param start the index of the first bit to set.
 * @param end   the index of the last bit to set.
 * @return      true on success, false otherwise.
 */
bool bmap_fill(bitmap_t* bmap, uint8_t value, size_t start, size_t end);

/**
 * Finds the index of the next bit with a given value. The
 * "hint" field of the bitmap is used as a starting location
 * for the search.
 *
 * @param bmap      the bitmap to use.
 * @param index     location where the index will be written on success.
 * @param desired   the desired value of the bit to find.
 * @param cnt       number of consecutive bits that shall have the desired value.
 * @param mul       the found index must be a multiple of this.
 * @param flags     controlling flags for the search (see BMAP_SRCH_*)
 * @return          true on success, false on failure (nothing found).
 */
bool bmap_search(bitmap_t* bmap, size_t* index, uint8_t desired, size_t cnt, size_t mul, uint32_t flags);

/**
 * Returns the filling degree of the bitmap in percent.
 *
 * @param bmap  the bitmap to inspect.
 * @return      the filling degree in percent.
 */
size_t bmap_fdeg(bitmap_t* bmap);
