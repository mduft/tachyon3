/* Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

/**
 * Compare a given portion of two given strings.
 * 
 * @param p the first string to compare
 * @param q the second string to compare
 * @param n the length of both strings to compare at most. 
 *          If either string is shorter, only this amount
 *          of characters will be compared.
 * @return 0 if both are equal, -1 if p is lexically "lower" than q,
 *         1 if q is lexically "higher" than p. 
 */
int8_t strncmp(char const *p, char const *q, size_t n);

/**
 * Compare two strings.
 *
 * @see strncmp
 */
int8_t strcmp(char const *p, char const *q);

/**
 * Copies a given portion of a given string to another string.
 *
 * @param s the target string to copy to.
 * @param t the source string to copy from.
 * @param n the amount of characters to copy.
 * @return a pointer to the target string
 */
char* strncpy(char *s, char const *t, size_t n);

/** 
 * Determine the length of the given string.
 *
 * @param s the string to measure length for.
 * @return the length in characters of the string (excluding trailing 0).
 */
size_t strlen(char const *s);

/**
 * Finds a given string's first occurance in another string.
 *
 * @param s the string that is searched
 * @param p the pattern that is searched for in s
 * @return either a point into s or NULL if not found.
 */
char const* strstr(char const *s, char const *p);

/**
 * Finds a given string's last occurance in another string.
 * Same as strstr but reverse.
 *
 * @see strstr
 */
char const* strrstr(char const *s, char const *p);
