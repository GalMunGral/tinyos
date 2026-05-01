#pragma once

#include "types.h"

void *memcpy(void *dst, const void *src, size_t n);
void *memset(void *dst, int c, size_t n);
int   strncmp(const char *a, const char *b, size_t n);