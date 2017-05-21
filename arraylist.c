#include "arraylist.h"

void alist_create(struct alist* list, u32 item_size, u32 capacity)
{
    list->cnt = 0;
    list->capacity = capacity;
    list->item_size = item_size;
    list->items = malloc(item_size * capacity);
}

void alist_add(struct alist *list, void *item)
{
    if ( list->cnt + 1 > list->capacity ) {
        list->capacity <<= 1;
        void *new_items = malloc(list->item_size * list->capacity);
        memcpy(new_items, list->items, list->cnt);
        free(list->items);
        list->items = new_items;
    }

    memcpy(list->items + list->cnt, item, list->item_size);
    ++(list->cnt);
}
