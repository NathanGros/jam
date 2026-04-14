#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "decompress.h"
#include "huffman_tree.h"

#define DEBUG 0

void print_usage() {
    printf("Usage: unjam <input> [output]\n");
}

int main(int argc, char *argv[]) {
    char input_file[512];
    char output_file[512];
    switch (argc) {
        case 2:
            strcpy(input_file, argv[1]);
            strcpy(output_file, argv[1]);
            // Remove .jam extension
            if (strcmp(output_file + strlen(output_file) - 4, ".jam") != 0) {
                printf("Error: Input file is not jammed.\n");
                print_usage();
                exit(1);
            }
            strcpy(output_file + strlen(output_file) - 4, "");
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
    if (DEBUG >= 1) {
        timer_start = clock();
        timer_current = timer_start;
    }

    // Retrieve encodings tree from the compressed file header
    huffman_tree_t *encodings_tree = retrieve_encodings(input_file);
    if (DEBUG == 2) {
        double elapsed = (double)(clock() - timer_current) / CLOCKS_PER_SEC;
        timer_current = clock();
        printf("Retrieved encodings in %fs\n", elapsed);
    }

    // Decompress and write to the output file
    write_decompressed_output(encodings_tree, input_file, output_file);
    if (DEBUG == 2) {
        double elapsed = (double)(clock() - timer_current) / CLOCKS_PER_SEC;
        timer_current = clock();
        printf("Decompressed in %fs\n", elapsed);
    }

    if (DEBUG >= 1) {
        double elapsed = (double)(clock() - timer_start) / CLOCKS_PER_SEC;
        printf("Total decompression time: %fs\n", elapsed);
    }
    return 0;
}
