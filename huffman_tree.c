#include "huffman_tree.h"

void print_tree(huffman_tree_t *tree) {
    if (tree == NULL) {
        printf("{}");
        return;
    }
    if (tree->character == NULL) {
        printf("{P:null, F:%d, left:", tree->occurences);
    } else {
        switch (tree->character[0]) {
            case '\0':
                printf("{P:'\\0', F:%d, left:", tree->occurences);
                break;
            case '\n':
                printf("{P:'\\n', F:%d, left:", tree->occurences);
                break;
            default:
                printf("{P:'%s', F:%d, left:", tree->character, tree->occurences);
                break;
        }
    }
    print_tree(tree->left);
    printf(", right:");
    print_tree(tree->right);
    printf("}");
}

huffman_tree_t *huffman_tree_create() {
    huffman_tree_t *tree = malloc(sizeof(huffman_tree_t));
    tree->character = NULL;
    tree->occurences = 0;
    tree->left = NULL;
    tree->right = NULL;
    return tree;
}

int compare_trees(huffman_tree_t *tree1, huffman_tree_t *tree2) {
    return tree2->occurences - tree1->occurences;
}

void insert_sorted(queue_t *trees, huffman_tree_t *tree) {
    if (queue_is_empty(trees)) {
        queue_push(trees, tree);
        return;
    }
    // Insert as head case
    if (compare_trees(queue_peek(trees), tree) <= 0) {
        queue_push_head(trees, tree);
        return;
    }
    // Insert as tail case
    if (compare_trees(queue_peek_tail(trees), tree) >= 0) {
        queue_push(trees, tree);
        return;
    }
    // Insert anywhere else case
    queue_t *new_queue = queue_create();
    while (1) {
        huffman_tree_t *compare_tree = queue_pop(trees);
        if (compare_trees(compare_tree, tree) > 0) {
            queue_push(new_queue, compare_tree);
            continue;
        }
        queue_push(new_queue, tree);
        queue_push_head(trees, compare_tree);
        break;
    }
    // "Merging" the queues together
    new_queue->tail->next = trees->head;
    trees->head = new_queue->head;
    free(new_queue);
}

queue_t *queue_to_trees(queue_t *character_occurences) {
    queue_t *trees = queue_create();
    while (!queue_is_empty(character_occurences)) {
        huffman_tree_t *leaf = malloc(sizeof(huffman_tree_t));
        occurences_t *occurences = queue_pop(character_occurences);
        leaf->character = occurences->character;
        leaf->occurences = occurences->occurences;
        leaf->left = NULL;
        leaf->right = NULL;
        free(occurences);
        insert_sorted(trees, leaf);
    }
    free(character_occurences);
    return trees;
}

huffman_tree_t *build_huffman_tree(queue_t *character_occurences) {
    queue_t *trees = queue_to_trees(character_occurences);
    while (trees->head != trees->tail) {
        huffman_tree_t *tree1 = queue_pop(trees);
        huffman_tree_t *tree2 = queue_pop(trees);
        huffman_tree_t *combined = malloc(sizeof(huffman_tree_t));
        combined->character = NULL;
        combined->occurences = tree1->occurences + tree2->occurences;
        combined->left = tree1;
        combined->right = tree2;
        insert_sorted(trees, combined);
    }
    huffman_tree_t *tree = queue_pop(trees);
    free(trees);
    return tree;
}

void build_encodings_rec(huffman_tree_t *tree, queue_t *encodings, char *buf, int buf_size) {
    if (tree->character != NULL) {
        character_encoding_t *encoding = malloc(sizeof(character_encoding_t));
        encoding->character = tree->character;
        free(tree);
        char *new_buf = malloc((buf_size) * sizeof(char));
        strcpy(new_buf, buf);
        encoding->encoding = new_buf;
        queue_push(encodings, encoding);
        return;
    }

    char new_buf_1[buf_size];
    strcpy(new_buf_1, buf);
    strcat(new_buf_1, "1");
    build_encodings_rec(tree->right, encodings, new_buf_1, buf_size + 1);

    char new_buf_0[buf_size];
    strcpy(new_buf_0, buf);
    strcat(new_buf_0, "0");
    build_encodings_rec(tree->left, encodings, new_buf_0, buf_size + 1);

    free(tree);
}

queue_t *build_encodings(huffman_tree_t *tree) {
    queue_t *encodings = queue_create();
    build_encodings_rec(tree, encodings, "", 1);
    return encodings;
}

void free_encodings_tree(huffman_tree_t *tree) {
    if (tree->character != NULL)
        free(tree->character);
    if (tree->left != NULL)
        free_encodings_tree(tree->left);
    if (tree->right != NULL)
        free_encodings_tree(tree->right);
    free(tree);
}
