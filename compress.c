#include "compress.h"

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
