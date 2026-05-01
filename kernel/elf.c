#include "elf.h"
#include "fs.h"
#include "mm.h"
#include "util.h"
#include "kprintf.h"

uint64_t elf_load(struct proc *p, const char *name) {
    struct fs_file *f = fs_open(name);
    if (!f) kpanic("elf_load: file not found\n");

    struct elf_hdr hdr;
    fs_read(f, &hdr, sizeof(hdr));

    if (*(uint32_t *)hdr.ident != ELF_MAGIC) kpanic("elf: bad magic\n");
    if (hdr.type    != ET_EXEC)              kpanic("elf: not executable\n");
    if (hdr.machine != EM_RISCV)             kpanic("elf: not RISC-V\n");

    for (int i = 0; i < hdr.phnum; i++) {
        struct elf_phdr ph;
        fs_seek(f, hdr.phoff + i * hdr.phentsize);
        fs_read(f, &ph, sizeof(ph));

        if (ph.type != PT_LOAD || ph.memsz == 0)
            continue;

        uint64_t pte_flags = PTE_U;
        if (ph.flags & PF_R) pte_flags |= PTE_R;
        if (ph.flags & PF_W) pte_flags |= PTE_W;
        if (ph.flags & PF_X) pte_flags |= PTE_X;

        uint64_t n_pages = (ph.memsz + PAGE_SIZE - 1) / PAGE_SIZE;
        for (uint64_t j = 0; j < n_pages; j++) {
            void *page = alloc_page();
            if (!page) kpanic("elf_load: out of memory\n");
            memset(page, 0, PAGE_SIZE);

            uint32_t file_off     = ph.offset + j * PAGE_SIZE;
            uint32_t file_remain  = ph.filesz > j * PAGE_SIZE ? ph.filesz - j * PAGE_SIZE : 0;
            if (file_remain > 0) {
                fs_seek(f, file_off);
                uint32_t to_read = file_remain < PAGE_SIZE ? file_remain : PAGE_SIZE;
                fs_read(f, page, to_read);
            }

            map_page(p->pagetable, ph.vaddr + j * PAGE_SIZE, kva2pa(page), pte_flags);
        }
    }

    return hdr.entry;
}