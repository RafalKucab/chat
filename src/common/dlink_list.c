#include "dlink_list.h"
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

typedef struct dlink_list_node dlink_list_node_t;

struct dlink_list_node {
  dlink_list_node_t *prev;
  dlink_list_node_t *next;
  void *value;
};

struct dlink_list {
  dlink_list_node_t *head;
  dlink_list_node_t *tail;
  size_t size;
  size_t elem_size;
};

dlink_list_t *dlink_list_init(size_t elem_size) {
  if (elem_size == 0) {
    return NULL;
  }

  dlink_list_t *dlink_list = (dlink_list_t *)malloc(sizeof(dlink_list_t));
  dlink_list->head = NULL;
  dlink_list->tail = NULL;
  dlink_list->size = 0;
  dlink_list->elem_size = elem_size;

  return dlink_list;
}

void dlink_list_free(dlink_list_t *list) {
  if (list == NULL) {
    return;
  }

  dlink_list_node_t *node = list->head;
  while (node != NULL) {
    dlink_list_node_t *next = node->next;
    free(node->value);
    free(node);
    node = next;
  }
  free(list);
}

size_t dlink_list_size(const dlink_list_t *list) {
  if (list == NULL) {
    return 0;
  }
  return list->size;
}

void *dlink_list_insert_at_end(dlink_list_t *list, void *value) {
  if (list == NULL || value == NULL) {
    return NULL;
  }

  dlink_list_node_t *new_node =
      (dlink_list_node_t *)malloc(sizeof(dlink_list_node_t));
  if (new_node == NULL) {
    return NULL;
  }

  void *new_value = (void *)malloc(list->elem_size);
  if (new_value == NULL) {
    free(new_node);
    return NULL;
  }
  memcpy(new_value, value, list->elem_size);

  if (list->tail == NULL) {
    list->tail = new_node;
    list->head = new_node;
    new_node->prev = NULL;
    new_node->next = NULL;
  } else {
    list->tail->next = new_node;
    new_node->prev = list->tail;
    new_node->next = NULL;
    list->tail = new_node;
  }
  new_node->value = new_value;
  list->size++;

  return new_value;
}

void *dlink_list_get_from(dlink_list_t *list, size_t idx) {
  if (list == NULL) {
    return NULL;
  }
  if (idx >= list->size) {
    return NULL;
  }

  dlink_list_node_t *node = list->head;
  for (size_t i = 0; i < idx; i++) {
    node = node->next;
  }

  return node->value;
}

static void unlink_node(dlink_list_t *list, dlink_list_node_t *node) {
  dlink_list_node_t *prev = node->prev;
  dlink_list_node_t *next = node->next;

  if (prev != NULL) {
    prev->next = next;
  } else {
    list->head = next;
  }

  if (next != NULL) {
    next->prev = prev;
  } else {
    list->tail = prev;
  }

  free(node->value);
  free(node);
  list->size--;
}

void dlink_list_delete_from(dlink_list_t *list, size_t idx) {
  if (list == NULL) {
    return;
  }
  if (idx >= list->size) {
    return;
  }

  dlink_list_node_t *node = list->head;
  for (size_t i = 0; i < idx; i++) {
    node = node->next;
  }

  unlink_node(list, node);
}

void dlink_list_delete(dlink_list_t *list, void *value,
                       int (*cmp)(void *, void *)) {
  if (list == NULL || cmp == NULL) {
    return;
  }

  for (dlink_list_node_t *node = list->head; node != NULL; node = node->next) {
    if (cmp(node->value, value) == 0) {
      unlink_node(list, node);
      return;
    }
  }
}

void *dlink_list_search(dlink_list_t *list, void *value,
                        int (*cmp)(void *, void *)) {
  if (list == NULL || cmp == NULL) {
    return NULL;
  }

  for (dlink_list_node_t *node = list->head; node != NULL; node = node->next) {
    if (cmp(node->value, value) == 0) {
      return node->value;
    }
  }
  return NULL;
}
