/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

typedef struct __listnode_t {
    struct __listnode_t* next;
    uintptr_t payload;
} listnode_t;

typedef struct {
    listnode_t* head;
} list_t;

list_t* list_new();
void list_delete(list_t* list);

bool list_add(list_t* list, uintptr_t item);
void list_remove(list_t* list, uintptr_t item);

