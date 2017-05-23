#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "list.h"

void list_new(struct list *list, int item_size, free_fn free_fn)
{
    list->length = 0;
    list->item_size = item_size;
    list->head = NULL;
    list->tail = NULL;
    list->free_fn = free_fn;
}

void list_free(struct list *list)
{
    struct listnode *current;

    while(list->head != NULL) {
        current = list->head;
        list->head = current->next;
        if(list->free_fn) {
            list->free_fn(current->data);
        }

        free(current->data);
        free(current);
    }
}

void list_prepend(struct list *list, void *item)
{
    struct listnode *node = malloc(sizeof(struct listnode));
    node->data = malloc(list->item_size);
    memcpy(node->data, item, list->item_size);

    node->next = list->head;
    list->head = node;

    // first node?
    if(list->tail == NULL) {
        list->tail = list->head;
    }

    ++(list->length);
}

void list_append(struct list *list, void *item)
{
    struct listnode *node = malloc(sizeof(struct listnode));
    node->data = malloc(list->item_size);
    node->next = NULL;

    memcpy(node->data, item, list->item_size);

    if(list->length == 0) {
        list->head = node;
        list->tail = node;
    } else {
        list->tail->next = node;
        list->tail = node;
    }

    ++(list->length);
}

void list_remove_after(struct list *list, struct listnode *node)
{
}

u32 list_size(struct list *list)
{
    return list->length;
}
