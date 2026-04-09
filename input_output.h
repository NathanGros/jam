#ifndef INPUT_OUTPUT_H
#define INPUT_OUTPUT_H

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "huffman_tree.h"
#include "queue.h"

queue_t *get_occurences(const char *input_file);
void write_compressed_output(queue_t *character_encodings, const char *input_file, const char *output_file);

huffman_tree_t *retrieve_encodings(const char *input_file);
void write_decompressed_output(huffman_tree_t *encodings_tree, const char *input_file, const char *output_file);

#endif
