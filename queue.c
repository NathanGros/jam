#include "queue.h"

queue_t *queue_create() {
    queue_t *q = malloc(sizeof(queue_t));
    q->head = NULL;
    q->tail = NULL;
    return q;
}

void queue_push(queue_t *q, void *data) {
    struct queue_node *new_node = malloc(sizeof(struct queue_node));
    new_node->data = data;
    new_node->next = NULL;
    if (q->head == NULL)
        q->head = new_node;
    if (q->tail != NULL)
        q->tail->next = new_node;
    q->tail = new_node;
}

void queue_push_head(queue_t *q, void *data) {
    if (queue_is_empty(q)) {
        queue_push(q, data);
        return;
    }
    struct queue_node *new_node = malloc(sizeof(struct queue_node));
    new_node->data = data;
    new_node->next = q->head;
    q->head = new_node;
}

void *queue_peek(queue_t *q) {
    if (q->head == NULL)
        return NULL;
    return q->head->data;
}

void *queue_peek_tail(queue_t *q) {
    if (queue_is_empty(q)) {
        return NULL;
    }
    return q->tail->data;
}

void *queue_pop(queue_t *q) {
    if (queue_is_empty(q))
        return NULL;
    void *data = q->head->data;
    struct queue_node *next_head = q->head->next;
    free(q->head);
    q->head = next_head;
    if (q->head == NULL)
        q->tail = NULL;
    return data;
}

bool queue_is_empty(queue_t *q) {
    return q->head == NULL;
}
