#pragma once

#include "types.h"
#include "proc.h"

#define ELF_MAGIC  0x464C457FU  // "\x7fELF" little-endian
#define ET_EXEC    2
#define EM_RISCV   243
#define PT_LOAD    1
#define PF_X       1
#define PF_W       2
#define PF_R       4

struct elf_hdr {
    uint8_t  ident[16];
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint64_t entry;
    uint64_t phoff;      // offset of program header table
    uint64_t shoff;
    uint32_t flags;
    uint16_t ehsize;
    uint16_t phentsize;  // size of one program header entry
    uint16_t phnum;      // number of program headers
    uint16_t shentsize;
    uint16_t shnum;
    uint16_t shstrndx;
};

struct elf_phdr {
    uint32_t type;
    uint32_t flags;    // PF_R / PF_W / PF_X
    uint64_t offset;   // offset in file
    uint64_t vaddr;    // virtual address to load at
    uint64_t paddr;
    uint64_t filesz;   // bytes in file (may be less than memsz for BSS)
    uint64_t memsz;    // bytes in memory
    uint64_t align;
};

uint64_t elf_load(struct proc *p, const char *name);
