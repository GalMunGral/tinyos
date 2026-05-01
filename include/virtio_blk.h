#pragma once

#include "types.h"

#define SECTOR_SIZE  512

void virtio_blk_init(void);

// Read nsectors * 512 bytes from sector into buf.
// buf must be in kernel VA space (kva2pa applies).
void virtio_blk_read(uint64_t sector, void *buf, uint32_t nsectors);
