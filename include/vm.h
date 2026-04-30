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

static inline uint64_t    pa2pte(uint64_t pa) { return (pa >> 12) << 10; }
static inline uint64_t    pte2pa(pte_t pte)   { return (pte >> 10) << 12; }
static inline void       *pa_to_va(uint64_t pa) { return (void *)(pa + KERNEL_OFFSET); }
static inline uint64_t    va_to_pa(void *va)    { return (uint64_t)va - KERNEL_OFFSET; }

static inline uint64_t pt_to_satp(pte_t *pt) { return (8UL << 60) | (va_to_pa(pt) >> 12); }

static inline void w_satp(uint64_t satp) { __asm__ volatile("csrw satp, %0" :: "r"(satp)); }
static inline void sfence_vma(void)      { __asm__ volatile("sfence.vma zero, zero" ::: "memory"); }

pte_t      *vm_new_table(void);
void        map_page(pte_t *pt, uint64_t va, uint64_t pa, uint64_t flags);
void        unmap_page(pte_t *pt, uint64_t va);