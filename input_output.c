#include "input_output.h"

# define IO_BUF_SIZE 512
char read_buffer[IO_BUF_SIZE];
int read_buffer_index;
int read_buffer_length;
char write_buffer[IO_BUF_SIZE];
int write_buffer_index;

void write_to_buffer(int fd_dst, char c) {
    write_buffer[write_buffer_index] = c;
    write_buffer_index += 1;
    // Write IO_BUF_SIZE chars at once when the write buffer is full
    if (write_buffer_index >= IO_BUF_SIZE) {
        write(fd_dst, write_buffer, IO_BUF_SIZE);
        write_buffer_index = 0;
    }
}

void flush_write_buffer(int fd_dst) {
    write(fd_dst, write_buffer, write_buffer_index);
}

int read_from_buffer(int fd_src, char *c) {
    if (read_buffer_index >= IO_BUF_SIZE) {
        read_buffer_length = read(fd_src, read_buffer, IO_BUF_SIZE);
        if (read_buffer_length < 0)
            return 0;
        read_buffer_index = 0;
    }
    if (read_buffer_length < IO_BUF_SIZE && read_buffer_index >= read_buffer_length)
        return 0;
    c[0] = read_buffer[read_buffer_index];
    read_buffer_index++;
    return 1;
}

int open_input_file(const char *src) {
    // Open src
    int fd_src = open(src, O_RDONLY);
    if (fd_src == -1) {
        printf("Error: cannot open file \"%s\"\n", src);
    }
    read_buffer_length = 0;
    read_buffer_index = IO_BUF_SIZE;
    return fd_src;
}

int open_output_file(const char *dst) {
    // Open or create dst
    int fd_dst = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_dst == -1) {
        printf("Error: cannot create \"%s\"\n", dst);
    }
    write_buffer_index = 0;
    return fd_dst;
}
