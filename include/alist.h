#ifndef VFAT_ALIST_H
#define VFAT_ALIST_H

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

void alist_create(struct alist* list, u32 item_size);
void alist_add(struct alist *list, void *item);
void alist_remove(struct alist *list, u32 idx);
u32 alist_count(struct alist *list);
void alist_get(struct alist *list, u32 idx, void *item);
void alist_destruct(struct alist *list);

#endif /* VFAT_ALIST_H */
