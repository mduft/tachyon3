/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

/**
 * Defines a spinlock. may contain more information
 * in the future.
 */
typedef struct {
    bool locked;    /**< whether the spinlock is locked. KEEP THIS FIRST */
} spinlock_t;

/**
 * Locks the spinlock. Blocks until the lock is held.
 *
 * @param lock  the spinlock to lock.
 */
void spl_lock(spinlock_t* lock);

/**
 * Tries to lock the spinlock once.
 *
 * @param lock  the spinlock to lock.
 */
bool spl_try_lock(spinlock_t* lock);

/**
 * Releases the lock on the spinlock.
 *
 * @param lock  the spinlock to unlock.
 */
void spl_unlock(spinlock_t* lock);

/**
 * Determines whether a given lock is currently locked.
 */
bool spl_locked(spinlock_t* lock);

/**
 * Determines whether a given lock is held by the current cpu.
 * (currently calls spl_locked, TODO: when SMP is here).
 */
bool spl_mine(spinlock_t* lock);

