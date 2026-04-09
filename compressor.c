#include <stdlib.h>
#include <stdio.h>

#include "input_output.h"
#include "queue.h"
#include "huffman_tree.h"

void print_usage() {
    printf("Usage: compress <infile> <outfile>\n");
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        print_usage();
        exit(1);
    }
    const char *input_file = argv[1];
    const char *output_file = argv[2];
    
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
