#ifndef INPUT_OUTPUT_H
#define INPUT_OUTPUT_H

#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

int open_input_file(const char *src);
int open_output_file(const char *dst);
void write_to_buffer(int fd_dst, char c);
void flush_write_buffer(int fd_dst);
int read_from_buffer(int fd_src, char *c);

#endif
