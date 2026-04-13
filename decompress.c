#include "decompress.h"

void grow_encodings_tree(huffman_tree_t **current_node, char *read_buf) {
    if (read_buf[0] == '0') {
        if ((*current_node)->left == NULL)
            (*current_node)->left = huffman_tree_create();
        *current_node = (*current_node)->left;
    } else {
        if ((*current_node)->right == NULL)
            (*current_node)->right = huffman_tree_create();
        *current_node = (*current_node)->right;
    }
}

void add_tree_encoding(huffman_tree_t *current_node, char c) {
    char *character_buf = malloc(3 * sizeof(char));
    character_buf[0] = c;
    character_buf[1] = '\0';
    if (character_buf[0] == '\0') {
        character_buf[0] = '\\';
        character_buf[1] = '0';
        character_buf[2] = '\0';
    }
    current_node->character = character_buf;
}

huffman_tree_t *retrieve_encodings(const char *input_file) {
    int fd_src = open_input_file(input_file);
    if (fd_src == -1)
        exit(1);

    huffman_tree_t *encodings_tree = huffman_tree_create();
    huffman_tree_t *current_node = encodings_tree;

    int nb_consecutive_stops = 1;
    int is_separator = 1;
    char character;
    char read_buf[1];
    while (1) {
        if (!read_from_buffer(fd_src, read_buf)) {
            break;
        }
        if (is_separator) { // Character
            character = read_buf[0];
        }
        if (!is_separator && read_buf[0] != '\0') { // Encoding sequence
            grow_encodings_tree(&current_node, read_buf);
        }
        if (!is_separator && nb_consecutive_stops < 1 && read_buf[0] == '\0') { // Separator
            add_tree_encoding(current_node, character);
            current_node = encodings_tree;
            is_separator = 1;
        } else {
            is_separator = 0;
        }

        if (read_buf[0] == '\0')
            nb_consecutive_stops++;
        else
            nb_consecutive_stops = 0;
        if (nb_consecutive_stops >= 3) {
            break;
        }
    }
    close(fd_src);
    return encodings_tree;
}

void process_bit(int fd_dst, int *found_eof, huffman_tree_t *encodings_tree, huffman_tree_t **current_node, int bit) {
    if (bit == 0)
        *current_node = (*current_node)->left;
    else
        *current_node = (*current_node)->right;
    if ((*current_node)->character != NULL) {
        if (strcmp((*current_node)->character, "\\0") == 0) {
            *found_eof = 1;
            flush_write_buffer(fd_dst);
        }
        write_to_buffer(fd_dst, (*current_node)->character[0]);
        *current_node = encodings_tree;
    }
}

void decode_byte(int fd_dst, int *found_eof, huffman_tree_t *encodings_tree, huffman_tree_t **current_node, unsigned char byte) {
    int byte_int = byte;
    int power = 128;
    for (int i = 0; i < 8; i++) {
        int bit = byte_int / power;
        process_bit(fd_dst, found_eof, encodings_tree, current_node, bit);
        if (*found_eof)
            break;
        byte_int = byte_int - bit * power;
        power = power / 2;
    }
}

void skip_header_read(int fd_src, char *read_buf) {
    int nb_consecutive_stops = 1;
    while (1) {
        if (!read_from_buffer(fd_src, read_buf))
            break;
        if (read_buf[0] == '\0')
            nb_consecutive_stops++;
        else
            nb_consecutive_stops = 0;
        if (nb_consecutive_stops >= 3) {
            break;
        }
    }
}

void write_decompressed_output(huffman_tree_t *encodings_tree, const char *input_file, const char *output_file) {
    // Open files
    int fd_src = open_input_file(input_file);
    if (fd_src == -1)
        exit(1);
    int fd_dst = open_output_file(output_file);
    if (fd_dst == -1) {
        close(fd_src);
        exit(1);
    }

    char read_buf[1];

    skip_header_read(fd_src, read_buf);

    // Go through the tree node by node, left when bit=0 and right when bit=1
    // If the tree node contains a character then we stop and start again from the root
    huffman_tree_t *current_node = encodings_tree;
    int found_eof = 0;
    while (!found_eof) {
        if (!read_from_buffer(fd_src, read_buf))
            break;
        unsigned char byte = read_buf[0];
        decode_byte(fd_dst, &found_eof, encodings_tree, &current_node, byte);
    }

    close(fd_src);
    close(fd_dst);
    free_encodings_tree(encodings_tree);
}
