#include "input_output.h"
#include "huffman_tree.h"
#include "queue.h"
#include <stdio.h>
#include <unistd.h>

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



/***************
 * Compression *
 ***************/

void add_new_occurence_count(queue_t *queue, char *c, int n) {
    occurences_t *entry = malloc(sizeof(occurences_t));
    entry->character = c;
    entry->occurences = n;
    queue_push(queue, entry);
}

queue_t *get_occurences(const char *input_file) {
    // Open src
    int fd_src = open_input_file(input_file);
    if (fd_src == -1)
        exit(1);

    // Count occurences
    int *occurences_table = malloc(256 * sizeof(int));
    unsigned char c;
    char read_buf[1];
    while (read_from_buffer(fd_src, read_buf)) {
        c = read_buf[0];
        occurences_table[(int) c] += 1;
    }
    close(fd_src);

    // Build queue
    queue_t *occurences = queue_create();
    for (int ch = 0; ch < 256; ch++) {
        if (occurences_table[ch] != 0) {
            char *character = malloc(3 * sizeof(char));
            character[0] = (char) ch;
            character[1] = '\0';
            add_new_occurence_count(occurences, character, occurences_table[ch]);
        }
    }
    free(occurences_table);

    // Adding \0 character
    char *character = malloc(3 * sizeof(char));
    character[0] = '\\';
    character[1] = '0';
    character[2] = '\0';
    add_new_occurence_count(occurences, character, 1);

    return occurences;
}

void write_encoding(character_encoding_t *encoding, int fd_dst) {
    char character[3];
    strcpy(character, encoding->character);
    if (strcmp(character, "\\0") == 0)
        character[0] = '\0';
    write_to_buffer(fd_dst, character[0]);
    char *character_encoding = encoding->encoding;
    for (int i = 0; i < strlen(character_encoding); i++) {
        write_to_buffer(fd_dst, character_encoding[i]);
    }
    write_to_buffer(fd_dst, '\0');
}

void write_encodings(queue_t *encodings, int fd_dst) {
    if (queue_is_empty(encodings)) {
        return;
    }
    queue_t *new_queue = queue_create();
    while (!queue_is_empty(encodings)) {
        character_encoding_t *encoding = queue_pop(encodings);
        write_encoding(encoding, fd_dst);
        queue_push(new_queue, encoding);
    }
    // "Merging" the queues together
    encodings->head = new_queue->head;
    encodings->tail = new_queue->tail;
    free(new_queue);
}

void write_header(queue_t *encodings, int fd_dst) {
    write_encodings(encodings, fd_dst);
    write_to_buffer(fd_dst, '\0');
    write_to_buffer(fd_dst, '\0');
}

char *get_encoding(queue_t *encodings, char *c, int *encoding_size) {
    if (queue_is_empty(encodings)) {
        *encoding_size = 0;
        return "";
    }

    char *encoding;
    queue_t *new_queue = queue_create();
    while (1) {
        if (queue_is_empty(encodings)) { // c is not in encodings (should not happen)
            encodings->head = new_queue->head;
            encodings->tail = new_queue->tail;
            free(new_queue);
            *encoding_size = 0;
            return "";
        }
        character_encoding_t *compare_encoding = queue_pop(encodings);
        if (strcmp(compare_encoding->character, c) != 0) {
            queue_push(new_queue, compare_encoding);
            continue;
        }
        encoding = compare_encoding->encoding;
        queue_push_head(encodings, compare_encoding);
        break;
    }
    // "Merging" the queues together
    if (queue_is_empty(new_queue)) {
        // If the encoding was the first element then new_queue is empty and can't be merged normally
        free(new_queue);
        *encoding_size = strlen(encoding);
        return encoding;
    }
    new_queue->tail->next = encodings->head;
    encodings->head = new_queue->head;
    free(new_queue);
    *encoding_size = strlen(encoding);
    return encoding;
}

void add_bit(char *byte_buf, int *byte_buf_size, char bit) {
    int byte = (int) *byte_buf;
    byte = 2*byte;
    if (bit == '1')
        byte++;
    *byte_buf = (char) byte;
    *byte_buf_size = *byte_buf_size + 1;
}

void free_encodings(queue_t *character_encodings) {
    while (!queue_is_empty(character_encodings)) {
        character_encoding_t *encoding = queue_pop(character_encodings);
        free(encoding->character);
        free(encoding->encoding);
        free(encoding);
    }
    free(character_encodings);
}

void convert_add_to_byte_buffer(int fd_dst, queue_t *character_encodings, char *to_encode, char *byte_buf, int *byte_buf_size) {
    int encoded_size;
    char *encoded = get_encoding(character_encodings, to_encode, &encoded_size);
    for (int i = 0; i < encoded_size; i++) {
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

void write_compressed_output(queue_t *character_encodings, const char *input_file, const char *output_file) {
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
    char char_buffer[3];
    char read_buf[1];
    while (read_from_buffer(fd_src, read_buf)) {
        char_buffer[0] = read_buf[0];
        char_buffer[1] = '\0';
        if (char_buffer[0] == '\0') { // char_buffer="\0"
            char_buffer[0] = '\\';
            char_buffer[1] = '0';
            char_buffer[2] = '\0';
        }
        convert_add_to_byte_buffer(fd_dst, character_encodings, char_buffer, &byte_buf, &byte_buf_size);
    }

    // Writing \0 character
    char eof_buf[3];
    eof_buf[0] = '\\';
    eof_buf[1] = '0';
    eof_buf[2] = '\0';
    convert_add_to_byte_buffer(fd_dst, character_encodings, eof_buf, &byte_buf, &byte_buf_size);

    // Flush remaining bits in the byte
    flush_buffers(fd_dst, &byte_buf, &byte_buf_size);

    close(fd_src);
    close(fd_dst);
    free_encodings(character_encodings);
}

/*****************
 * Decompression *
 *****************/

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
