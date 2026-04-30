#include "vm.h"
#include "mm.h"
#include "kprintf.h"

static void zero_page(void *p) {
    uint64_t *q = (uint64_t *)p;
    for (int i = 0; i < PAGE_SIZE / 8; i++)
        q[i] = 0;
}

// Walk the 3-level page table to find the level-0 PTE for va.
// alloc=true: create missing intermediate tables; alloc=false: return 0 if missing.
static pte_t *walk(pte_t *pt, uint64_t va, bool alloc) {
    for (int level = 2; level > 0; level--) {
        pte_t *pte = &pt[(va >> (12 + 9 * level)) & 0x1FF];
        if (*pte & PTE_V) {
            pt = (pte_t *)pa_to_va(pte2pa(*pte));
        } else {
            if (!alloc) return 0;
            void *pg = alloc_page();
            if (!pg) kpanic("walk: out of memory\n");
            zero_page(pg);
            *pte = pa2pte(va_to_pa(pg)) | PTE_V;
            pt = (pte_t *)pg;
        }
    }
    return &pt[(va >> 12) & 0x1FF];
}

void map_page(pte_t *pt, uint64_t va, uint64_t pa, uint64_t flags) {
    pte_t *pte = walk(pt, va, true);
    *pte = pa2pte(pa) | flags | PTE_V | PTE_A | PTE_D;
}

void unmap_page(pte_t *pt, uint64_t va) {
    pte_t *pte = walk(pt, va, false);
    if (pte) *pte = 0;
}

pte_t *vm_new_table(void) {
    void *pg = alloc_page();
    if (!pg) kpanic("vm_new_table: out of memory\n");
    zero_page(pg);
    return (pte_t *)pg;
}