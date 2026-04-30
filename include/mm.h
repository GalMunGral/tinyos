#pragma once

#include "types.h"

#define PAGE_SIZE 4096

void  mm_init(void);
void *alloc_page(void);
void  free_page(void *page);