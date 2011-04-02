/* Copyright (c) 2010 by Markus Duft <mduft@gentoo.org>
 * This file is part of the 'tachyon' operating system. */

#include "list.h"
#include "kheap.h"
#include "mem.h"
#include "log.h"

list_t* list_new() {
    list_t* list = (list_t*)kheap_alloc(sizeof(list_t));
    memset(list, 0, sizeof(list));

    return list;
}

list_t* list_delete(list_t* list) {
    if(!list)
        return NULL;

    list_node_t* node = list->head;

    while(node != NULL) {
        list_node_t* next_node = node->next;
        kheap_free(node);
        node = next_node;
    }

    kheap_free(list);
    return NULL;
}

void list_add(list_t* list, void const* data) {
    if(!list) {
        error("NULL list to add to.\n");
        return;
    }

    list_node_t* new_node = (list_node_t*)kheap_alloc(sizeof(list_node_t));
    memset(new_node, 0, sizeof(new_node));

    new_node->data = data;

    if(!list->tail) {
        list->head = new_node;
    } else {
        list->tail->next = new_node;
    }

    list->tail = new_node;
    list->size++;
}

void list_remove(list_t* list, list_node_t* item) {
    if(!list) {
        error("NULL list to remove from.\n");
        return;
    }

    list_node_t* node = list->head;

    while(node != NULL) {
        if(node->next == item) {
            node->next = node->next->next;
            list->size--;

            kheap_free(item);

            return;
        }
    }

    error("failed to remove item from list\n");
}

list_node_t* list_begin(list_t* list) {
    if(list)
        return list->head;

    return NULL;
}

size_t list_size(list_t* list) {
    if(!list)
        return 0;

    return list->size;
}
