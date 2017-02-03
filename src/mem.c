/* Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
 * This file is part of the 'tachyon' operating system. */

#include <mem.h>

void* memset(void *dst, uint8_t c, size_t n) {
    register uint8_t* p = (uint8_t*)dst;

    while(n--) { *p++ = c; }

    return dst;
}

int8_t memcmp(void const *v1, void const *v2, size_t n) {
	size_t i;

	for (i = 0; i < n; i++) {
		if (((uint8_t*)v1)[i] != ((uint8_t*)v2)[i]) {
			return 1;
		}
	}

	return 0;
}

void* memcpy(void *dst, void const *src, size_t n) {
	size_t i;

	for (i = 0; i < n; i++) {
		((uint8_t*)dst)[i] = ((uint8_t*)src)[i];
	}

	return dst;
}

void* memmove(void *dst, void const *src, size_t n) {
	register size_t i;
	uint8_t *d = (uint8_t*) dst;
	uint8_t *s = (uint8_t*) src;

	if (src == dst) {
		return dst;
	}
	else if (src > dst) {
		for (i = 0; i < n; i++) {
			d[i] = s[i];
		}
	}
	else if (src < dst) {
		for (i = 0; i < n; i++) {
			d[n - i - 1] = s[n - i - 1];
		}
	}

	return dst;

}

