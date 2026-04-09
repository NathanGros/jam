#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
    
    // Build queue<char, int> from input
    queue_t *occurences = get_occurences(input_file);

    // Build Huffman tree from queue
    huffman_tree_t *tree = build_huffman_tree(occurences);

    // Build queue<character, encoding> from Huffman tree
    queue_t *character_encodings = build_encodings(tree);

    // Generate output: encoding table + encoded text
    write_compressed_output(character_encodings, input_file, output_file);

    return 0;
}
