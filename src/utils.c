#include "utils.h"

#include <stdio.h>
#include <stdlib.h>

char *read_file(const char *filename) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror(filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    /* +1 for null-terminator */
    char *buffer = malloc(file_size + 1);
    if (!buffer) {
        fprintf(stderr, "malloc failed\n");
        return NULL;
    }

    if (fread(buffer, 1, file_size, file) != file_size) {
        perror(filename);
        return NULL;
    }

    /* Null-terminate the string */
    buffer[file_size] = '\0';
    return buffer;
}
