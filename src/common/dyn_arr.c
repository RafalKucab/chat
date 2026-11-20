#include "dyn_arr.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "dyn_arr.h"

struct dyn_arr {
  void **data;
  size_t size;
  size_t capacity;
  size_t elem_size;
};

dyn_arr_t *dyn_arr_init(size_t capacity, size_t elem_size) {
  if (capacity == 0 || elem_size == 0) {
    return NULL;
  }
  dyn_arr_t *array = malloc(sizeof(dyn_arr_t));
  if (array == NULL) {
    return NULL;
  }

  array->data = malloc(capacity * sizeof(void *));
  if (array->data == NULL) {
    free(array);
    return NULL;
  }
  array->size = 0;
  array->capacity = capacity;
  array->elem_size = elem_size;

  return array;
}

void dyn_arr_free(dyn_arr_t *array) {
  if (array == NULL)
    return;
  for (size_t i = 0; i < array->size; i++) {
    free(array->data[i]);
  }

  free(array->data);
  free(array);
}

size_t dyn_arr_size(const dyn_arr_t *array) {
  if (array == NULL) {
    return 0;
  }
  return array->size;
}

size_t dyn_arr_capacity(const dyn_arr_t *array) {
  if (array == NULL) {
    return 0;
  }
  return array->capacity;
}

void *dyn_arr_push(dyn_arr_t *array, const void *item) {
  if (array == NULL) {
    return NULL;
  }
  if (item == NULL) {
    return NULL;
  }
  if (array->capacity == array->size) {
    size_t new_cap = 2 * array->capacity;
    void **new_data = realloc(array->data, new_cap * sizeof(void *));
    if (new_data == NULL) {
      return NULL;
    }
    array->data = new_data;
    array->capacity = new_cap;
  }

  void *new_item = malloc(array->elem_size);
  if (new_item == NULL) {
    return NULL;
  }
  memcpy(new_item, item, array->elem_size);

  array->data[array->size] = new_item;
  array->size++;

  return new_item;
}

void *dyn_arr_get(const dyn_arr_t *array, size_t index) {
  if (array == NULL) {
    return NULL;
  }
  if (index >= array->size) {
    return NULL;
  }

  return array->data[index];
}
