#ifndef CHAT_ARRAY_H
#define CHAT_ARRAY_H

#include <stddef.h>

typedef struct {
  int *data;
  size_t size;
  size_t capacity;
} chat_list_t;

chat_list_t *init_arr(size_t capacity);

#endif
