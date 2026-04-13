#include "compress.h"
#include <time.h>

void get_occurences(const char *input_file, int *occurences) {
    // Open src
    int fd_src = open_input_file(input_file);
    if (fd_src == -1)
        exit(1);

    // Count occurences
    unsigned char c;
    char read_buf[1];
    while (read_from_buffer(fd_src, read_buf)) {
        c = read_buf[0];
        occurences[(int) c] += 1;
    }
    close(fd_src);

    unsigned char eof = '\0';
    occurences[(int) eof] += 1;
}

void write_encoding(char c, char *encoding, int fd_dst) {
    write_to_buffer(fd_dst, c);
    for (int i = 0; i < strlen(encoding); i++) {
        write_to_buffer(fd_dst, encoding[i]);
    }
    write_to_buffer(fd_dst, '\0');
}

void write_encodings(char **encodings, int fd_dst) {
    for (int i = 0; i < 256; i++) {
        if (encodings[i] == NULL)
            continue;
        unsigned char c = i;
        write_encoding(c, encodings[i], fd_dst);
    }
}

void write_header(char **encodings, int fd_dst) {
    write_encodings(encodings, fd_dst);
    write_to_buffer(fd_dst, '\0');
    write_to_buffer(fd_dst, '\0');
}

void add_bit(char *byte_buf, int *byte_buf_size, char bit) {
    int byte = (int) *byte_buf;
    byte = 2*byte;
    if (bit == '1')
        byte++;
    *byte_buf = (char) byte;
    *byte_buf_size = *byte_buf_size + 1;
}

void convert_add_to_byte_buffer(int fd_dst, char **character_encodings, unsigned char to_encode, char *byte_buf, int *byte_buf_size) {
    char *encoded = character_encodings[(int) to_encode];
    for (int i = 0; i < strlen(encoded); i++) {
        add_bit(byte_buf, byte_buf_size, encoded[i]);
        if (*byte_buf_size == 8) {
            *byte_buf_size = 0;
            write_to_buffer(fd_dst, byte_buf[0]);
        }
    }
}

void flush_buffers(int fd_dst, char *byte_buf, int *byte_buf_size) {
    while (*byte_buf_size != 8) {
        add_bit(byte_buf, byte_buf_size, '0');
    }
    write_to_buffer(fd_dst, byte_buf[0]);
    flush_write_buffer(fd_dst);
}

void write_compressed_output(char **character_encodings, const char *input_file, const char *output_file) {
    // Open files
    int fd_src = open_input_file(input_file);
    if (fd_src == -1)
        exit(1);
    int fd_dst = open_output_file(output_file);
    if (fd_dst == -1) {
        close(fd_src);
        exit(1);
    }

    // Write encodings table header
    write_header(character_encodings, fd_dst);

    char byte_buf;
    int byte_buf_size = 0;
    unsigned char c;
    char read_buf[1];
    while (read_from_buffer(fd_src, read_buf)) {
        c = read_buf[0];
        convert_add_to_byte_buffer(fd_dst, character_encodings, c, &byte_buf, &byte_buf_size);
    }

    // Writing \0 character
    convert_add_to_byte_buffer(fd_dst, character_encodings, '\0', &byte_buf, &byte_buf_size);

    // Flush remaining bits in the byte
    flush_buffers(fd_dst, &byte_buf, &byte_buf_size);

    close(fd_src);
    close(fd_dst);
    for (int i = 0; i < 256; i++) {
        if (character_encodings[i] != NULL)
            free(character_encodings[i]);
    }
    free(character_encodings);
}
