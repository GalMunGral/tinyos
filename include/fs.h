#pragma once

#include "types.h"
#include "virtio_blk.h"

#define FS_MAGIC       0x54494E59  // "TINY"
#define FS_NAME_MAX    20
#define FS_BLOCK_SIZE  4096
#define FS_INODES      112         // (4096 - 512) / 32
#define FS_BLOCK_DATA  (FS_BLOCK_SIZE - 4)

// First 512 bytes of disk; inode table fills bytes 512-4095
struct fs_super {
    uint32_t magic;
    uint32_t inode_count;
    uint32_t block_count;
    uint32_t free_head;    // first free data block, 0 = none
    uint8_t  _pad[496];
};

#define INODE_VALID  (1 << 0)  // slot is in use

// 32 bytes; free slot when !(flags & INODE_VALID)
struct fs_inode {
    uint32_t flags;
    uint32_t data_head;
    uint32_t size;
    char     name[FS_NAME_MAX];
};

// 4KB; next == 0 means end of chain
struct fs_block {
    uint32_t next;
    uint8_t  data[FS_BLOCK_DATA];
};

static inline uint32_t fs_block_to_sector(uint32_t block) {
    return (FS_BLOCK_SIZE / SECTOR_SIZE) + block * (FS_BLOCK_SIZE / SECTOR_SIZE);
}

struct fs_file {
    struct fs_inode *inode;
    uint32_t         pos;
};

void            fs_init(void);
struct fs_file *fs_open(const char *name);
void            fs_seek(struct fs_file *f, uint32_t offset);
uint32_t        fs_read(struct fs_file *f, void *buf, uint32_t size);
