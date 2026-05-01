#include "fs.h"
#include "virtio_blk.h"
#include "util.h"
#include "kprintf.h"

#define INODES_OFFSET  512

static struct fs_super super;
static struct fs_inode inodes[FS_INODES];
static struct fs_block block;
static struct fs_file  open_file;

static void read_block(uint32_t blk) {
    virtio_blk_read(fs_block_to_sector(blk), &block, FS_BLOCK_SIZE / SECTOR_SIZE);
}

void fs_init(void) {
    static uint8_t buf[FS_BLOCK_SIZE];
    virtio_blk_read(0, buf, FS_BLOCK_SIZE / SECTOR_SIZE);

    struct fs_super *sb = (struct fs_super *)buf;
    if (sb->magic != FS_MAGIC)
        kpanic("fs: bad magic\n");

    super = *sb;
    memcpy(inodes, buf + INODES_OFFSET, sizeof(inodes));
    kprintf("fs: %u data blocks\n", super.block_count);
}

struct fs_file *fs_open(const char *name) {
    for (int i = 0; i < FS_INODES; i++) {
        if (!(inodes[i].flags & INODE_VALID)) continue;
        if (strncmp(inodes[i].name, name, FS_NAME_MAX) == 0) {
            open_file.inode = &inodes[i];
            open_file.pos   = 0;
            return &open_file;
        }
    }
    return NULL;
}

void fs_seek(struct fs_file *f, uint32_t offset) {
    f->pos = offset;
}

uint32_t fs_read(struct fs_file *f, void *buf, uint32_t size) {
    if (f->pos + size > f->inode->size)
        size = f->inode->size - f->pos;

    uint32_t skip = f->pos / FS_BLOCK_DATA;
    uint32_t cur  = f->inode->data_head;
    for (uint32_t s = 0; s < skip && cur != 0; s++) {
        read_block(cur);
        cur = block.next;
    }

    uint32_t remaining = size;
    uint32_t block_off = f->pos % FS_BLOCK_DATA;
    uint8_t *dst       = (uint8_t *)buf;

    while (remaining > 0 && cur != 0) {
        read_block(cur);
        uint32_t avail = FS_BLOCK_DATA - block_off;
        uint32_t n     = remaining < avail ? remaining : avail;
        memcpy(dst, block.data + block_off, n);
        dst       += n;
        remaining -= n;
        block_off  = 0;
        cur        = block.next;
    }

    uint32_t n_read = size - remaining;
    f->pos += n_read;
    return n_read;
}
