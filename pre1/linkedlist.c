/**
 * Doubly Linked List Implementation
 */

#include "list.h"
#include "printing.h"

#include <stdlib.h>
#include <string.h>

typedef struct lnode lnode_t;
struct lnode {
    void    *item;
    lnode_t *prev;
    lnode_t *next;
};

struct list {
    lnode_t *head;
    lnode_t *tail;
    size_t   length;
    cmp_fn   cmpfn;
};

struct list_iter {
    list_t  *list;
    lnode_t *current;
};

static lnode_t *newnode(void *item) {
    lnode_t *node = malloc(sizeof(lnode_t));
    if (!node) return NULL;
    node->item = item;
    node->prev = NULL;
    node->next = NULL;
    return node;
}

list_t *list_create(cmp_fn cmpfn) {
    list_t *list = malloc(sizeof(list_t));
    if (!list) return NULL;
    list->head   = NULL;
    list->tail   = NULL;
    list->length = 0;
    list->cmpfn  = cmpfn;
    return list;
}

void list_destroy(list_t *list, free_fn item_free) {
    lnode_t *node = list->head;
    while (node) {
        lnode_t *next = node->next;
        if (item_free) item_free(node->item);
        free(node);
        node = next;
    }
    free(list);
}

size_t list_length(list_t *list) {
    return list->length;
}

int list_addfirst(list_t *list, void *item) {
    lnode_t *node = newnode(item);
    if (!node) return -1;

    node->next = list->head;
    node->prev = NULL;

    if (list->head) {
        list->head->prev = node;
    } else {
        list->tail = node;
    }
    list->head = node;
    list->length++;
    return 0;
}

int list_addlast(list_t *list, void *item) {
    lnode_t *node = newnode(item);
    if (!node) return -1;

    node->prev = list->tail;
    node->next = NULL;

    if (list->tail) {
        list->tail->next = node;
    } else {
        list->head = node;
    }
    list->tail = node;
    list->length++;
    return 0;
}

void *list_popfirst(list_t *list) {
    if (!list->head) {
        pr_error("list_popfirst called on empty list\n");
        exit(EXIT_FAILURE);
    }
    lnode_t *node = list->head;
    void    *item = node->item;

    list->head = node->next;
    if (list->head) {
        list->head->prev = NULL;
    } else {
        list->tail = NULL;
    }
    free(node);
    list->length--;
    return item;
}

void *list_poplast(list_t *list) {
    if (!list->tail) {
        pr_error("list_poplast called on empty list\n");
        exit(EXIT_FAILURE);
    }
    lnode_t *node = list->tail;
    void    *item = node->item;

    list->tail = node->prev;
    if (list->tail) {
        list->tail->next = NULL;
    } else {
        list->head = NULL;
    }
    free(node);
    list->length--;
    return item;
}

int list_contains(list_t *list, void *item) {
    lnode_t *node = list->head;
    while (node) {
        if (list->cmpfn(node->item, item) == 0) return 1;
        node = node->next;
    }
    return 0;
}

/* --- Merge Sort for linked list --- */

static lnode_t *split_list(lnode_t *head) {
    lnode_t *slow = head;
    lnode_t *fast = head->next;

    while (fast && fast->next) {
        slow = slow->next;
        fast = fast->next->next;
    }
    lnode_t *second = slow->next;
    slow->next = NULL;
    if (second) second->prev = NULL;
    return second;
}

static lnode_t *merge_sorted(lnode_t *a, lnode_t *b, cmp_fn cmpfn) {
    if (!a) return b;
    if (!b) return a;

    lnode_t *head;
    if (cmpfn(a->item, b->item) <= 0) {
        head = a; a = a->next;
    } else {
        head = b; b = b->next;
    }
    head->prev = NULL;
    head->next = NULL;

    lnode_t *tail = head;
    while (a && b) {
        lnode_t *chosen;
        if (cmpfn(a->item, b->item) <= 0) {
            chosen = a; a = a->next;
        } else {
            chosen = b; b = b->next;
        }
        chosen->prev = tail;
        chosen->next = NULL;
        tail->next   = chosen;
        tail         = chosen;
    }
    lnode_t *rest = a ? a : b;
    while (rest) {
        rest->prev = tail;
        tail->next = rest;
        tail       = rest;
        rest       = rest->next;
    }
    tail->next = NULL;
    return head;
}

static lnode_t *mergesort(lnode_t *head, cmp_fn cmpfn) {
    if (!head || !head->next) return head;
    lnode_t *second = split_list(head);
    head   = mergesort(head, cmpfn);
    second = mergesort(second, cmpfn);
    return merge_sorted(head, second, cmpfn);
}

void list_sort(list_t *list) {
    if (list->length < 2) return;
    list->head = mergesort(list->head, list->cmpfn);
    lnode_t *node = list->head;
    while (node->next) node = node->next;
    list->tail = node;
}

list_iter_t *list_createiter(list_t *list) {
    list_iter_t *iter = malloc(sizeof(list_iter_t));
    if (!iter) return NULL;
    iter->list    = list;
    iter->current = list->head;
    return iter;
}

void list_destroyiter(list_iter_t *iter) {
    free(iter);
}

int list_hasnext(list_iter_t *iter) {
    return iter->current != NULL;
}

void *list_next(list_iter_t *iter) {
    if (!iter->current) return NULL;
    void *item    = iter->current->item;
    iter->current = iter->current->next;
    return item;
}

void list_resetiter(list_iter_t *iter) {
    iter->current = iter->list->head;
}
