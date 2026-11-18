#include "list.h"
#include <stdlib.h>

chat_list_t *init_arr(size_t capacity) {
  chat_list_t *list = malloc(sizeof(chat_list_t));
  if (list == NULL) {
    return NULL;
  }

  list->data = NULL;
  if (capacity > 0) {
    list->data = malloc(capacity * sizeof(int));
    if (list->data == NULL) {
      free(list);
      return NULL;
    }
  }

  list->size = 0;
  list->capacity = capacity;

  return list;
}
