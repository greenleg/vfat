#ifndef VFAT_ARRAY_LIST_H
#define VFAT_ARRAY_LIST_H

#include <stdlib.h>
#include <string.h>

#include "common.h"

struct alist
{
    void *items;
    u32 cnt;
    u32 capacity;
    u32 item_size;
};

void alist_create(struct alist* list, u32 item_size, u32 capacity);
void alist_add(struct alist *list, void *item);

#endif /* VFAT_ARRAY_LIST_H */
