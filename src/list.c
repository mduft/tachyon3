/* Copyright (c) 2011 by Markus Duft <markus.duft@ssi-schaefer.com>
 * This file is part of the 'tachyon' operating system. */

#include "list.h"
#include "kheap.h"
#include "mem.h"
#include "log.h"

list_t* list_new() {
    list_t* list = (list_t*)kheap_alloc(sizeof(list_t));
    memset(list, 0, sizeof(list_t));

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
    list_insert(list, list->tail, data);
}

void list_insert(list_t* list, list_node_t* node, void const* data) {
    if(!list) {
        error("NULL list to add to.\n");
        return;
    }

    list_node_t* new_node = (list_node_t*)kheap_alloc(sizeof(list_node_t));
    memset(new_node, 0, sizeof(list_node_t));

    new_node->data = data;

    if(!node) {
        list->head = new_node;

        if(!list->tail)
            list->tail = new_node;
    } else {
        if(list->tail == node) {
            list->tail = new_node;
        } else {
            new_node->next = node->next;
        }

        node->next = new_node;
    }

    list->size++;
}

void list_remove(list_t* list, void const* item) {
    if(!list) {
        error("NULL list to remove from.\n");
        return;
    }

    list_node_t* prev = NULL;
    list_node_t* node = list->head;

    while(node) {
        if(node->data == item) {
            if(prev) {
                prev->next = node->next;
            } else {
                list->head = node->next;
            }

            if(list->tail == node) {
                list->tail = prev ? prev : list->head;
            }

            kheap_free(node);
            list->size--;
            return;
        }

        prev = node;
        node = node->next;
    }

    warn("failed to remove item from list\n");
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

void list_clear(list_t* list) {
    while(list && list->size > 0) {
        list_remove(list, list->head->data);
    }
}
