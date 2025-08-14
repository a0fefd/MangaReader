//
// Created by nbigi0 on 14/08/2025.
//

#include "util.h"

void read_until_eof(FILE *file, size_t *out_size, char *buffer_in) {
    size_t capacity = 8192;  // Start with 8 KB
    size_t size = 0;
    char *buffer = malloc(capacity + 1);  // +1 for optional null terminator
    if (!buffer) return;

    while (!feof(file) && !ferror(file)) {
        if (size == capacity) {
            capacity *= 2;
            char *new_buf = realloc(buffer, capacity + 1);
            if (!new_buf) {
                free(buffer);
                return;
            }
            buffer = new_buf;
        }

        size_t bytes_read = fread(buffer + size, 1, capacity - size, file);
        size += bytes_read;
    }

    buffer[size] = '\0';  // Optional null terminator
    if (out_size) *out_size = size;
    memcpy(buffer_in, buffer, size+1);
}

int strsplice(char *dest, const char *src, const char *start, const char *end)
{
    memset(dest, '\0', strlen(dest));
    char dest2[MAX_DATA_SIZE];
    memcpy(dest2, src, strlen(src));

    char *pos = strstr(src, start);
    if (!pos)
        return -1;

    memcpy(dest2, pos, strlen(pos));
    pos = strstr(dest2, end);
    if (!pos)
        return -2;

    memset(dest2 + (pos - dest2), '\0', strlen(pos));

    memset(dest, '\0', strlen(dest));
    memccpy(dest, dest2, '\0', strlen(dest2));

    return 0;
}

int strsplice_single(char *str, const char *start, const char *end)
{
    if (!str)
        return -1;

    size_t size = strlen(str);
    if (!size)
        return -2;

    char str2[MAX_DATA_SIZE] = {'\0'};
    memccpy(str2, str, '\0', size);
    const int retval = strsplice(str, str2, "<div id=\"chapter", "</div>");
    return retval;
}

uint32_t strtok_counter(const char *str, const char *delim)
{
    uint32_t counter = 0;

    // char *str2 = (char *)str;

    char *pos = strstr(str, delim);
    while (pos)
    {
        counter++;
        pos = strstr(pos + 1, delim);
    }

    return counter;
}