//
// Created by nbigi0 on 14/08/2025.
//

#ifndef NB_UTIL
#define NB_UTIL

#define MAX_DATA_SIZE (1<<17)

#ifndef NB_UTIL_INCL
#define NB_UTIL_INCL
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#endif

#ifndef _GCC_WRAP_STDINT_H
#include <stdint.h>
#endif

void read_until_eof(FILE *file, size_t *out_size, char *buffer_in);
int strsplice(char *dest, const char *src, const char *start, const char *end);
int strsplice_single(char *str, const char *start, const char *end);
uint32_t strtok_counter(const char *str, const char *delim);

#endif