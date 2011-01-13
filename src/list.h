/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

/**
 * Structure defining a single list node in the single linked list.
 */
typedef struct __listnode_t {
    struct __listnode_t* next;  /**< pointer to the following structure, or NULL if end */
    uintptr_t payload;          /**< the actual data, needs to be casted. */
} listnode_t;

/**
 * Structure denoting the actual list itself. This exists for the
 * sole purpose to represent empty lists (head == NULL), which would
 * not be possible when directly using listnode_t as list type.
 */
typedef struct {
    listnode_t* head;   /**< the head of the list, NULL if empty */
} list_t;

/**
 * Allocates and returns a new list_t. Initially, the list is empty
 *
 * @return  a newly allocated list_t.
 */
list_t* list_new();

/**
 * Deletes a list_t, that has been created with list_new(). Additionally
 * removes all items from the list (freeing the infratsructural memory).
 *
 * @param list  the list to destroy.
 */
void list_delete(list_t* list);

/**
 * Adds a new item to the list, allocating a listnode_t structure as
 * needed.
 *
 * @param list  the list to add the item to.
 * @param item  the actual data to insert into the list.
 * @return      true on success, false on error.
 */
bool list_add(list_t* list, uintptr_t item);

/**
 * Removes an existing item from the list. This also frees the listnode_t
 * associated with the item.
 *
 * @param list  the list to remove the item from.
 * @param item  the item to search for.
 */
void list_remove(list_t* list, uintptr_t item);

