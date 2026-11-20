#ifndef DLINK_LIST_H
#define DLINK_LIST_H

#include <stddef.h>

typedef struct dlink_list dlink_list_t;

dlink_list_t *dlink_list_init(size_t capacity, size_t elem_size);

void dlink_list_free(dlink_list_t *list);

size_t dlink_list_size(const dlink_list_t *list);

// void *dlink_list_push(dlink_list_t *list, const void *item);

// void *dlink_list_get(const dlink_list_t *list, size_t index);

#endif
