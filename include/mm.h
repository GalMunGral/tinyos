#pragma once

#include "types.h"

void  mm_init(void);
void *alloc_page(void);
void  free_page(void *page);