#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "input_output.h"
#include "huffman_tree.h"

void print_usage() {
    printf("Usage: decompress <infile> <outfile>\n");
}

int main(int argc, char *argv[]) {
    int print_time = 1;
    clock_t timer_start;

    if (argc != 3) {
        print_usage();
        exit(1);
    }
    const char *input_file = argv[1];
    const char *output_file = argv[2];
    
    if (print_time)
        timer_start = clock();
    // Retrieve encodings tree from the compressed file header
    huffman_tree_t *encodings_tree = retrieve_encodings(input_file);

    // Decompress and write to the output file
    write_decompressed_output(encodings_tree, input_file, output_file);
    if (print_time) {
        double elapsed = (double)(clock() - timer_start) / CLOCKS_PER_SEC;
        printf("Decompression time: %fs\n", elapsed);
    }
    return 0;
}
