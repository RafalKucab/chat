#include "list.h"
#include "common.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

chat_list_t *arr_init(size_t capacity) {
  if (capacity == 0) {
    return NULL;
  }
  chat_list_t *list = malloc(sizeof(chat_list_t));
  if (list == NULL) {
    return NULL;
  }

  list->data = malloc(capacity * sizeof(msg_t *));
  if (list->data == NULL) {
    free(list);
    return NULL;
  }
  list->size = 0;
  list->capacity = capacity;

  return list;
}

void arr_free(chat_list_t *arr) {
  if (arr == NULL)
    return;

  free(arr->data);
  free(arr);
}

msg_t *arr_push(chat_list_t *arr, msg_t *item) {
  if (arr == NULL) {
    return NULL;
  }
  if (item == NULL) {
    return NULL;
  }
  if (arr->capacity == arr->size) {
    arr->capacity = 2 * arr->capacity;
    arr->data = realloc(arr->data, arr->capacity * sizeof(msg_t *));
  }

  msg_t *new_msg = (msg_t *)malloc(sizeof(msg_t));

  strcpy(new_msg->user, item->user);
  strcpy(new_msg->message, item->message);
  arr->data[arr->size] = new_msg;
  arr->size++;

  return arr->data[arr->size - 1];
}

msg_t *arr_get(chat_list_t *arr, size_t index) {
  if (arr == NULL || arr->data == NULL) {
    return NULL;
  }
  if (index < 0 || index >= arr->size) {
    return NULL;
  }

  return arr->data[index];
}
