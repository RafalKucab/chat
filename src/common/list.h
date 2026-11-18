#ifndef CHAT_ARRAY_H
#define CHAT_ARRAY_H

#include <stddef.h>

#include "common.h"

typedef struct {
  msg_t **data;
  size_t size;
  size_t capacity;
} chat_list_t;

chat_list_t *arr_init(size_t capacity);

void arr_free(chat_list_t *arr);

msg_t *arr_push(chat_list_t *arr, msg_t *item);

msg_t *arr_get(chat_list_t *arr, size_t index);

#endif
