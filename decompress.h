#ifndef DECOMPRESS_H
#define DECOMPRESS_H

#include "input_output.h"
#include "huffman_tree.h"
#include "queue.h"

huffman_tree_t *retrieve_encodings(const char *input_file);
void write_decompressed_output(huffman_tree_t *encodings_tree, const char *input_file, const char *output_file);

#endif
