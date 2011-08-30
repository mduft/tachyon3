/* Copyright (c) 2011 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include <string.h>

int8_t strncmp(char const *p, char const *q, size_t n) {
    register size_t i;

	for (i = 0; i < n; i++) {
		if (p[i] == q[i]) {
			if (p[i] == '\0') return 0;
			continue;
		}
		if (p[i] == '\0') return -1;
		if (q[i] == '\0') return 1;
		if (p[i] < q[i]) return -1;
		else return 1;
	}

	return 0;
}

int8_t strcmp(char const *p, char const *q) {
	register size_t i;

	for (i = 0;; i++) {
		if (p[i] == q[i]) {
			if (p[i] == '\0') return 0;
			continue;
		}
		if (p[i] == '\0') return -1;
		if (q[i] == '\0') return 1;
		if (p[i] < q[i]) return -1;
		else return 1;
	}
}

char* strncpy(char *s, char const *t, size_t n) {
    register size_t i;

	for (i = 0; t[i] != '\0' && i < n; i++) {
		s[i] = t[i];
	}
	for (; i < n; i++) {
		s[i] = '\0';
	}

	return s;
}

size_t strlen(char const *s) {
    register size_t i;

	for (i = 0; s[i]; i++);

	return i;
}
