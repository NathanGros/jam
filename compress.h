#ifndef COMPRESS_H
#define COMPRESS_H

#include "input_output.h"
#include "huffman_tree.h"
#include "queue.h"

queue_t *get_occurences(const char *input_file);
void write_compressed_output(queue_t *character_encodings, const char *input_file, const char *output_file);

#endif
