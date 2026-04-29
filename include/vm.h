#pragma once

#include "types.h"

#define KERNEL_OFFSET  0xFFFFFFC000000000UL  // VA = PA + KERNEL_OFFSET for all kernel memory

// PTE flag bits
#define PTE_V  (1UL << 0)   // valid
#define PTE_R  (1UL << 1)   // readable
#define PTE_W  (1UL << 2)   // writable
#define PTE_X  (1UL << 3)   // executable
#define PTE_U  (1UL << 4)   // user-accessible
#define PTE_A  (1UL << 6)   // accessed
#define PTE_D  (1UL << 7)   // dirty

typedef uint64_t  pte_t;
typedef pte_t    *pagetable_t;   // one page = 512 pte_t entries

static inline uint64_t    pa2pte(uint64_t pa) { return (pa >> 12) << 10; }
static inline uint64_t    pte2pa(pte_t pte)   { return (pte >> 10) << 12; }
static inline void       *pa_to_va(uint64_t pa) { return (void *)(pa + KERNEL_OFFSET); }
static inline uint64_t    va_to_pa(void *va)    { return (uint64_t)va - KERNEL_OFFSET; }

pagetable_t vm_new_table(void);
void        map_page(pagetable_t pt, uint64_t va, uint64_t pa, uint64_t flags);
void        unmap_page(pagetable_t pt, uint64_t va);