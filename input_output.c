#include "input_output.h"
#include "huffman_tree.h"
#include <unistd.h>

/***************
 * Compression *
 ***************/

int open_input_file(const char *src) {
    // Open src
    int fd_src = open(src, O_RDONLY);
    if (fd_src == -1) {
        printf("Error: cannot open file \"%s\"\n", src);
    }
    return fd_src;
}

int open_output_file(const char *dst) {
    // Open or create dst
    int fd_dst = open(dst, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_dst == -1) {
        printf("Error: cannot create \"%s\"\n", dst);
    }
    return fd_dst;
}

void add_new_entry(queue_t *queue, char *c) {
    occurences_t *entry = malloc(sizeof(occurences_t));
    entry->character = c;
    entry->occurences = 1;
    queue_push(queue, entry);
}

void add_occurence(queue_t *occurences, char *c) {
    if (queue_is_empty(occurences)) {
        add_new_entry(occurences, c);
        return;
    }
    queue_t *new_queue = queue_create();
    while (1) {
        if (queue_is_empty(occurences)) { // c is not already in occurences
            add_new_entry(occurences, c);
            break;
        }
        occurences_t *compare_occurence = queue_pop(occurences);
        if (strcmp(compare_occurence->character, c) != 0) {
            queue_push(new_queue, compare_occurence);
            continue;
        }
        free(c);
        compare_occurence->occurences++;
        queue_push_head(occurences, compare_occurence);
        break;
    }
    // "Merging" the queues together
    if (queue_is_empty(new_queue)) {
        // If the occurence was the first element then new_queue is empty and can't be merged normally
        free(new_queue);
        return;
    }
    new_queue->tail->next = occurences->head;
    occurences->head = new_queue->head;
    free(new_queue);
}

queue_t *get_occurences(const char *input_file) {
    // Open src
    int fd_src = open_input_file(input_file);
    if (fd_src == -1)
        exit(1);

    // Count occurences
    queue_t *occurences = queue_create();
    char buffer[3];
    while (read(fd_src, buffer, 1) > 0) {
        buffer[1] = '\0';
        if (buffer[0] == '\0') { // buffer="\0"
            buffer[0] = '\\';
            buffer[1] = '0';
            buffer[2] = '\0';
        }
        char *character = malloc(3 * sizeof(char));
        strcpy(character, buffer);
        add_occurence(occurences, character);
    }

    // Adding \0 character
    char *character = malloc(3 * sizeof(char));
    character[0] = '\\';
    character[1] = '0';
    character[2] = '\0';
    add_occurence(occurences, character);

    close(fd_src);
    return occurences;
}

void write_encoding(character_encoding_t *encoding, int fd_dst) {
    char character[3];
    strcpy(character, encoding->character);
    if (strcmp(character, "\\0") == 0) {
        char buffer[1];
        buffer[0] = '\0';
        write(fd_dst, buffer, 1);
    } else {
        write(fd_dst, character, 1);
    }
    char *character_encoding = encoding->encoding;
    write(fd_dst, character_encoding, strlen(character_encoding));
    char separation[1];
    separation[0] = '\0';
    write(fd_dst, separation, 1);
}

void write_encodings(queue_t *encodings, int fd_dst) {
    if (queue_is_empty(encodings)) {
        return;
    }

    queue_t *new_queue = queue_create();
    while (1) {
        if (queue_is_empty(encodings)) { // End
            break;
        }
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
    char header_end[2];
    header_end[0] = '\0';
    header_end[1] = '\0';
    write(fd_dst, header_end, 2);
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
            char byte_to_write[1];
            byte_to_write[0] = *byte_buf;
            write(fd_dst, byte_to_write, 1);
        }
    }
}

void flush_write_byte_buffer(int fd_dst, char *byte_buf, int *byte_buf_size) {
    while (*byte_buf_size != 0) {
        add_bit(byte_buf, byte_buf_size, '0');
        if (*byte_buf_size == 8) {
            *byte_buf_size = 0;
            char byte_to_write[1];
            byte_to_write[0] = *byte_buf;
            write(fd_dst, byte_to_write, 1);
        }
    }
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

    // Convert then write encodings by adding converted bits one by one into a char (byte), then write the char when it is "full"
    char byte_buf;
    int byte_buf_size = 0;
    char buffer[3];
    while (read(fd_src, buffer, 1) > 0) {
        buffer[1] = '\0';
        if (buffer[0] == '\0') { // buffer="\0"
            buffer[0] = '\\';
            buffer[1] = '0';
            buffer[2] = '\0';
        }
        convert_add_to_byte_buffer(fd_dst, character_encodings, buffer, &byte_buf, &byte_buf_size);
    }

    // Writing \0 character
    char eof_buf[3];
    eof_buf[0] = '\\';
    eof_buf[1] = '0';
    eof_buf[2] = '\0';
    convert_add_to_byte_buffer(fd_dst, character_encodings, eof_buf, &byte_buf, &byte_buf_size);

    // Flush remaining bits in the byte
    flush_write_byte_buffer(fd_dst, &byte_buf, &byte_buf_size);

    close(fd_src);
    close(fd_dst);
    free_encodings(character_encodings);
}

/*****************
 * Decompression *
 *****************/

# define IO_BUF_SIZE 512

huffman_tree_t *retrieve_encodings(const char *input_file) {
    int fd_src = open_input_file(input_file);
    if (fd_src == -1)
        exit(1);

    huffman_tree_t *encodings_tree = huffman_tree_create();
    huffman_tree_t *current_node = encodings_tree;

    // Read header
    int nb_consecutive_stops = 1;
    int is_separator = 1;
    char buffer[IO_BUF_SIZE];
    int buffer_index = IO_BUF_SIZE;
    char character;
    int end_of_header = 0;
    while (!end_of_header) {
        if (buffer_index >= IO_BUF_SIZE) {
            buffer_index = 0;
            if (read(fd_src, buffer, IO_BUF_SIZE) < 0) {
                break;
            }
        }

        if (is_separator) { // Character
            character = buffer[buffer_index];
        }
        if (!is_separator && buffer[buffer_index] != '\0') { // Encoding sequence
            if (buffer[buffer_index] == '0') {
                if (current_node->left == NULL)
                    current_node->left = huffman_tree_create();
                current_node = current_node->left;
            } else {
                if (current_node->right == NULL)
                    current_node->right = huffman_tree_create();
                current_node = current_node->right;
            }
        }
        if (!is_separator && nb_consecutive_stops < 1 && buffer[buffer_index] == '\0') { // Separator
            char *character_buf = malloc(3 * sizeof(char));
            character_buf[0] = character;
            character_buf[1] = '\0';
            if (character_buf[0] == '\0') {
                character_buf[0] = '\\';
                character_buf[1] = '0';
                character_buf[2] = '\0';
            }
            current_node->character = character_buf;
            current_node = encodings_tree;
            is_separator = 1;
        } else {
            is_separator = 0;
        }

        if (buffer[buffer_index] == '\0')
            nb_consecutive_stops++;
        else
            nb_consecutive_stops = 0;
        if (nb_consecutive_stops >= 3) {
            end_of_header = 1;
        }
        buffer_index++;
    }
    close(fd_src);
    return encodings_tree;
}

void flush_write_buffer(int fd_dst, char *write_buffer, int nb_to_write) {
    write(fd_dst, write_buffer, nb_to_write);
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

    int end_of_header = 0;
    int nb_consecutive_stops = 1;
    char header_buffer[IO_BUF_SIZE];
    int header_buffer_index = IO_BUF_SIZE;
    char read_buffer[IO_BUF_SIZE];
    int read_buffer_index = IO_BUF_SIZE;
    char write_buffer[IO_BUF_SIZE];
    int write_buffer_index = 0;

    // Skip header
    while (!end_of_header) {
        if (header_buffer_index >= IO_BUF_SIZE) {
            header_buffer_index = 0;
            if (read(fd_src, header_buffer, IO_BUF_SIZE) < 0) {
                break;
            }
        }
        if (header_buffer[header_buffer_index] == '\0')
            nb_consecutive_stops++;
        else
            nb_consecutive_stops = 0;
        header_buffer_index++;
        
        // Fill read_buffer with the end of header_buffer (after the stop) and complete up to IO_BUF_SIZE chars
        if (nb_consecutive_stops >= 3) {
            for (int i = 0; i < IO_BUF_SIZE - header_buffer_index; i++) {
                read_buffer[i] = header_buffer[header_buffer_index + i];
            }
            read_buffer_index = IO_BUF_SIZE - header_buffer_index;
            read(fd_src, read_buffer+read_buffer_index, header_buffer_index);
            read_buffer_index = 0;
            end_of_header = 1;
        }
    }

    // Go through the tree node by node, left when bit=0 and right when bit=1
    // If the tree node contains a character then we stop and start again from the root
    huffman_tree_t *current_node = encodings_tree;
    int found_eof = 0;
    while (!found_eof) {
        // Read 512 chars when the end of the read buffer is reached
        if (read_buffer_index >= IO_BUF_SIZE) {
            read_buffer_index = 0;
            if (read(fd_src, read_buffer, IO_BUF_SIZE) < 0) {
                break;
            }
        }
        unsigned char byte = read_buffer[read_buffer_index];
        int byte_int = byte;
        int power = 128;
        for (int i = 0; i < 8; i++) {
            int bit = byte_int / power;
            if (bit == 0)
                current_node = current_node->left;
            else
                current_node = current_node->right;
            if (current_node->character != NULL) {
                // printf("%c", current_node->character[0]);
                if (strcmp(current_node->character, "\\0") == 0) {
                    found_eof = 1;
                    flush_write_buffer(fd_dst, write_buffer, write_buffer_index);
                    break;
                }
                write_buffer[write_buffer_index] = current_node->character[0];
                write_buffer_index++;
                // Write 512 chars when the write buffer is full
                if (write_buffer_index >= IO_BUF_SIZE) {
                    write(fd_dst, write_buffer, IO_BUF_SIZE);
                    write_buffer_index = 0;
                }
                current_node = encodings_tree;
            }
            byte_int = byte_int - bit * power;
            power = power / 2;
        }
        read_buffer_index++;
    }

    close(fd_src);
    close(fd_dst);
    free_encodings_tree(encodings_tree);
}
