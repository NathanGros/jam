#ifndef HUFFMAN_TREE_H
#define HUFFMAN_TREE_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "queue.h"

struct huffman_tree_node {
    char *character;
    int occurences;
    struct huffman_tree_node *left;
    struct huffman_tree_node *right;
};

typedef struct huffman_tree_node huffman_tree_t;

void print_tree(huffman_tree_t *tree);
huffman_tree_t *huffman_tree_create();
huffman_tree_t *build_huffman_tree(queue_t *character_occurences);
queue_t *build_encodings(huffman_tree_t *tree);
void free_encodings_tree(huffman_tree_t *tree);

#endif
