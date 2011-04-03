/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#pragma once

#include "tachyon.h"

/**
 * Defines the layout of a single list node.
 */
typedef struct _tag_list_node_t {
    void const* data;         /**< the associated data */
    struct _tag_list_node_t* next; /**< pointer to the next node, or NULL */
} list_node_t;

/**
 * The actual list descriptor. It holds a pointer to
 * both the head, and the tail (for fast insertions).
 */
typedef struct _tag_list_t {
    list_node_t* head;  /**< the head of the list. */
    list_node_t* tail;  /**< the tail of the list. */
    size_t size;        /**< the number of items. */
} list_t;

/**
 * Creates a new list, effectively allocating a list_t.
 *
 * @return  the new list.
 */
list_t* list_new();

/**
 * Deletes a list, freeing all memory associated with
 * list management structures. The actual data is left
 * alone.
 *
 * @param   list the list to destroy.
 * @return  always NULL.
 */
list_t* list_delete(list_t* list);

/**
 * Adds a new node at the end of the list, containing
 * the specified data.
 *
 * @param list  the list to add a node to.
 * @parma data  the data to insert into the node.
 */
void list_add(list_t* list, void const* data);

/**
 * Removes a given item from the list.
 *
 * @param list  the list to remove the item from.
 * @param node  the node to remove.
 */
void list_remove(list_t* list, void const* data);

/**
 * Start iterating over list_node_t's.
 *
 * @param list  the list to get the head from.
 * @return      pointer to the first item in the list.
 */
list_node_t* list_begin(list_t* list);

/**
 * Retrieve the current item count of the list.
 *
 * @param list  the list to check.
 * @return      the size of the list.
 */
size_t list_size(list_t* list);
