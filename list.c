#include "list.h"

#include <stdio.h>
#include <stdlib.h>

#include "players.h"

typedef struct Node {
  Item data;
  struct Node *next;
} * node;

struct List {
  node head;
};

static node new_node(Item data, node next) {
  node n = (node)malloc(sizeof(struct Node));

  if (!n) exit(0);

  n->data = data;
  n->next = next;

  return n;
}

List put(List list, Item data) {
  if (!list) {
    list = (List)malloc(sizeof(struct List));
    if (!list) exit(0);
    list->head = NULL;
  }

  list->head = new_node(data, list->head);
  printf("put %d\n", ((player *)data)->id);
  return list;
}

// TODO: implement getting id
Item pop(List list) {
  if (list == NULL || list->head == NULL) return NULL;

  node head = list->head;
  Item data = head->data;

  head->data = NULL;

  if (!list->head->next || !list->head->next->data) return data;

  list->head = head->next;

  return data;
}

// TODO: check if necessary to free data
void delete_list(List list) {
  node next_node;
  if (!list) return;

  for (node n = list->head; n != NULL; n = next_node) {
    next_node = n->next;
    free(n);
  }

  if (list) free(list);
}

void printlist(List list) {
  node next_node;
  if (!list) return;
  printf("\n");
  for (node n = list->head; n != NULL; n = next_node) {
    printf("%p: %d, ", (void *)n, n->data ? (*(player *)(n->data)).id : -1);
    next_node = n->next;
  }
  printf("\n");
  printf("\n");
}

node get_head(List list) { return list ? list->head : NULL; }

Item get_data(node node) { return node ? node->data : NULL; }

node next(node node) { return node ? node->next : NULL; }

// TODO: pass compare function
int delete_node(List list, int fd) {
  node prev = list->head;
  printf("deleting: %d\n", fd);
  for (node n = prev; n != NULL; n = n->next) {
    if (((player *)(n->data))->id == fd) {
      prev->next = n->next;
      if (prev == n) list->head = n->next;
      free(n);
      return 1;
    }
    prev = n;
  }
  return 0;
}

Item get_item(List list, int fd) {
  for (node n = list->head; n != NULL; n = n->next)
    if (((player *)(n->data))->id == fd) {
      return n->data;
    }

  return NULL;
}
