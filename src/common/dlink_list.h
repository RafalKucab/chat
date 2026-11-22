#ifndef DLINK_LIST_H
#define DLINK_LIST_H

#include <stddef.h>

typedef struct dlink_list dlink_list_t;
typedef struct dlink_list_node dlink_list_node_t;

dlink_list_t *dlink_list_init(size_t elem_size);

void dlink_list_free(dlink_list_t *list);

size_t dlink_list_size(const dlink_list_t *list);

void *dlink_list_insert_at_end(dlink_list_t *list, void *value);

void *dlink_list_get_from(dlink_list_t *list, size_t idx);

void dlink_list_delete_from(dlink_list_t *list, size_t idx);

void dlink_list_delete(dlink_list_t *list, void *value,
                       int (*cmp)(void *, void *));

void *dlink_list_search(dlink_list_t *list, void *value,
                        int (*cmp)(void *, void *));

// not implemented
dlink_list_node_t *insert_at_begin(dlink_list_t *list, void *value);

dlink_list_node_t *insert_at(dlink_list_t *list, void *value, size_t idx);

dlink_list_node_t *delete_from_end(dlink_list_t *list, void *value);

dlink_list_node_t *delete_from_begin(dlink_list_t *list, void *value);

dlink_list_node_t *delete_from(dlink_list_t *list, void *value, size_t idx);

#endif
