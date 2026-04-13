#ifndef COMPRESS_H
#define COMPRESS_H

#include "input_output.h"
#include "huffman_tree.h"
#include "queue.h"

void get_occurences(const char *input_file, int *occurences);
void write_compressed_output(char **character_encodings, const char *input_file, const char *output_file);

#endif
