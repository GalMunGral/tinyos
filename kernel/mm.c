#include "mm.h"
#include "vm.h"
#include "kprintf.h"

#define RAM_END   (KERNEL_OFFSET + 0x88000000UL)  // high VA of end of 128 MB RAM

extern char __kernel_end[];

struct freepage {
    struct freepage *next;
};

static struct freepage *free_list;

void free_page(void *page) {
    struct freepage *p = (struct freepage *)page;
    p->next = free_list;
    free_list = p;
}

void *alloc_page(void) {
    if (!free_list)
        return 0;
    struct freepage *p = free_list;
    free_list = p->next;
    return p;
}

void mm_init(void) {
    uintptr_t start = (uintptr_t)__kernel_end;
    start = (start + PAGE_SIZE - 1) & ~(PAGE_SIZE - 1);  // round up to page boundary

    uintptr_t end = RAM_END;
    uint64_t n = 0;

    for (uintptr_t addr = start; addr + PAGE_SIZE <= end; addr += PAGE_SIZE) {
        free_page((void *)addr);
        n++;
    }

    kprintf("mm: %d pages free (%d KB)\n", n, n * 4);
}
