#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "compress.h"
#include "huffman_tree.h"

#define DEBUG 0

void print_usage() {
    printf("Usage: jam <input> [output]\n");
}

int main(int argc, char *argv[]) {
    char input_file[512];
    char output_file[512];
    switch (argc) {
        case 2:
            strcpy(input_file, argv[1]);
            strcpy(output_file, argv[1]);
            strcat(output_file, ".jam");
            break;
        case 3:
            strcpy(input_file, argv[1]);
            strcpy(output_file, argv[2]);
            break;
        default:
            print_usage();
            exit(1);
    }
    
    clock_t timer_start;
    clock_t timer_current;
    if (DEBUG) {
        timer_start = clock();
        timer_current = timer_start;
    }

    // Build occurences from input
    int *occurences = malloc(256 * sizeof(int));
    for (int i = 0; i < 256; i++) {
        occurences[i] = 0;
    }
    get_occurences(input_file, occurences);
    if (DEBUG) {
        double elapsed = (double)(clock() - timer_current) / CLOCKS_PER_SEC;
        timer_current = clock();
        printf("Got occurences queue in %fs\n", elapsed);
    }

    // Build Huffman tree from queue
    huffman_tree_t *tree = build_huffman_tree(occurences);
    if (DEBUG) {
        double elapsed = (double)(clock() - timer_current) / CLOCKS_PER_SEC;
        timer_current = clock();
        printf("Built Huffman tree in %fs\n", elapsed);
    }

    // Build queue<character, encoding> from Huffman tree
    char **character_encodings = malloc(256 * sizeof(char*));
    for (int i = 0; i < 256; i++) {
        character_encodings[i] = NULL;
    }
    build_encodings(tree, character_encodings);
    if (DEBUG) {
        double elapsed = (double)(clock() - timer_current) / CLOCKS_PER_SEC;
        timer_current = clock();
        printf("Built encodings in %fs\n", elapsed);
    }

    // Generate output: encoding table + encoded text
    write_compressed_output(character_encodings, input_file, output_file);
    if (DEBUG) {
        double elapsed = (double)(clock() - timer_current) / CLOCKS_PER_SEC;
        timer_current = clock();
        printf("wrote output in %fs\n", elapsed);
    }

    if (DEBUG) {
        double elapsed = (double)(clock() - timer_start) / CLOCKS_PER_SEC;
        printf("Total: %fs\n", elapsed);
    }
    return 0;
}
