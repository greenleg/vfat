#ifndef VFAT_LIST_H
#define VFAT_LIST_H

#include "common.h"
#include <stdbool.h>

/* A common function used to free malloc'd objects */
typedef void (*free_fn)(void *);

struct listnode
{
    void *data;
    struct listnode *prev;
    struct listnode *next;
};

struct list
{
    u32 length;
    u32 item_size;
    struct listnode *head;
    struct listnode *tail;
    struct listnode *current;
    free_fn free_fn;
};

void list_new(struct list *list, int item_size, free_fn free_fn);
void list_free(struct list *list);

void list_prepend(struct list *list, void *item);
void list_append(struct list *list, void *item);
void list_remove(struct list *list, struct listnode *prev, struct listnode *node);
u32 list_size(struct list *list);

void listiter_reset(struct list *list);
bool listiter_hasnext(struct list *list);
void listiter_remove(struct list *list);
void *listiter_current(struct list *list);

#endif /* VFAT_LIST_H */
