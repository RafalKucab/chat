#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

#include <stddef.h>

typedef struct dyn_arr dyn_arr_t;

dyn_arr_t *dyn_arr_init(size_t capacity, size_t elem_size);

void dyn_arr_free(dyn_arr_t *array);

size_t dyn_arr_size(const dyn_arr_t *array);

size_t dyn_arr_capacity(const dyn_arr_t *array);

void *dyn_arr_push(dyn_arr_t *array, const void *item);

void *dyn_arr_get(const dyn_arr_t *array, size_t index);

#endif
