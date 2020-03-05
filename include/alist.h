#ifndef VFAT_ALIST_H
#define VFAT_ALIST_H

#include <stdlib.h>
#include <string.h>

#include "common.h"

struct alist
{
    void *items;
    uint32_t cnt;
    uint32_t capacity;
    uint32_t item_size;
};

void alist_create(struct alist* list, uint32_t item_size);
void alist_add(struct alist *list, void *item);
void alist_remove(struct alist *list, uint32_t idx);
uint32_t alist_count(struct alist *list);
void alist_get(struct alist *list, uint32_t idx, void *item);
void alist_destruct(struct alist *list);

#endif /* VFAT_ALIST_H */
