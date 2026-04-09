#ifndef QUEUE_H
#define QUEUE_H

#include <stdlib.h>
#include <stdbool.h>

struct queue_node {
    void *data;
    struct queue_node *next;
};

typedef struct {
    struct queue_node *head;
    struct queue_node *tail;
} queue_t;

typedef struct {
    char *character;
    int occurences;
} occurences_t;

typedef struct {
    char *character;
    char *encoding;
} character_encoding_t;

queue_t *queue_create();
void queue_push(queue_t *q, void *data);
void queue_push_head(queue_t *q, void *data);
void *queue_peek(queue_t *q);
void *queue_peek_tail(queue_t *q);
void *queue_pop(queue_t *q);
bool queue_is_empty(queue_t *q);

#endif
