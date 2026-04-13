#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "input_output.h"
#include "queue.h"
#include "huffman_tree.h"

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
    
    int print_time = 1;
    clock_t timer_start;
    clock_t timer_current;
    if (print_time) {
        timer_start = clock();
        timer_current = timer_start;
    }

    // Build queue<char, int> from input
    queue_t *occurences = get_occurences(input_file);
    if (print_time) {
        double elapsed = (double)(clock() - timer_current) / CLOCKS_PER_SEC;
        timer_current = clock();
        printf("Got occurences queue in %fs\n", elapsed);
    }

    // Build Huffman tree from queue
    huffman_tree_t *tree = build_huffman_tree(occurences);
    if (print_time) {
        double elapsed = (double)(clock() - timer_current) / CLOCKS_PER_SEC;
        timer_current = clock();
        printf("Built Huffman tree in %fs\n", elapsed);
    }

    // Build queue<character, encoding> from Huffman tree
    queue_t *character_encodings = build_encodings(tree);
    if (print_time) {
        double elapsed = (double)(clock() - timer_current) / CLOCKS_PER_SEC;
        timer_current = clock();
        printf("Built encodings in %fs\n", elapsed);
    }

    // Generate output: encoding table + encoded text
    write_compressed_output(character_encodings, input_file, output_file);
    if (print_time) {
        double elapsed = (double)(clock() - timer_current) / CLOCKS_PER_SEC;
        timer_current = clock();
        printf("wrote output in %fs\n", elapsed);
    }

    return 0;
}
