#!/usr/bin/env python3
# Usage: mkfs.py <output.img> [file ...]

import sys
import os
import struct
from typing import BinaryIO, NamedTuple

BLOCK_SIZE   = 4096
SECTOR_SIZE  = 512
SUPER_SIZE   = SECTOR_SIZE
INODE_SIZE   = 32
NAME_MAX     = 20
CHUNK_SIZE   = BLOCK_SIZE - 4
INODE_COUNT  = (BLOCK_SIZE - SUPER_SIZE) // INODE_SIZE
FS_MAGIC     = 0x54494E59
INODE_VALID  = 1 << 0
DISK_SIZE    = 4 * 1024 * 1024   # 4 MB
TOTAL_BLOCKS = DISK_SIZE // BLOCK_SIZE

class FileEntry(NamedTuple):
    name        : str
    size        : int
    first_block : int
    nblocks     : int
    path        : str

def block_offset(idx: int) -> int:
    return BLOCK_SIZE + idx * BLOCK_SIZE

# superblock

def write_super(f: BinaryIO, nblocks_used: int) -> None:
    f.write(struct.pack('<IIII', FS_MAGIC, INODE_COUNT, TOTAL_BLOCKS - 1, nblocks_used))
    f.seek(SUPER_SIZE)

# inodes

def write_inodes(f: BinaryIO, layout: list[FileEntry]) -> None:
    for i, e in enumerate(layout):
        f.write(struct.pack('<III', INODE_VALID, e.first_block, e.size))
        f.write(e.name.encode()[:NAME_MAX-1] + b'\x00')
        f.seek(SUPER_SIZE + (i + 1) * INODE_SIZE)

# data blocks

def write_file_blocks(f: BinaryIO, layout: list[FileEntry]) -> None:
    for e in layout:
        with open(e.path, 'rb') as src:
            for j in range(e.nblocks):
                next_block = e.first_block + j + 1 if j + 1 < e.nblocks else 0
                f.seek(block_offset(e.first_block + j))
                f.write(struct.pack('<I', next_block))
                f.write(src.read(CHUNK_SIZE))

def write_free_blocks(f: BinaryIO, nblocks_used: int) -> None:
    for idx in range(nblocks_used, TOTAL_BLOCKS):
        next_block = idx + 1 if idx + 1 < TOTAL_BLOCKS else 0
        f.seek(block_offset(idx))
        f.write(struct.pack('<I', next_block))
        f.seek(block_offset(idx + 1) - 1)
        f.write(b'\x00')

# layout

def layout_files(paths: list[str]) -> tuple[list[FileEntry], int]:
    result     : list[FileEntry] = []
    next_block : int             = 1
    for path in paths:
        name    = os.path.basename(path)
        size    = os.path.getsize(path)
        nblocks = (size + CHUNK_SIZE - 1) // CHUNK_SIZE
        first   = next_block if nblocks > 0 else 0
        result.append(FileEntry(name, size, first, nblocks, path))
        next_block += nblocks
    return result, next_block

def main() -> None:
    out_path = sys.argv[1]
    paths    = sys.argv[2:]

    if len(paths) > INODE_COUNT:
        sys.exit(f'too many files: max {INODE_COUNT}')

    layout, nblocks_used = layout_files(paths)
    if nblocks_used >= TOTAL_BLOCKS:
        sys.exit('disk too small for files')

    with open(out_path, 'wb') as f:
        write_super(f, nblocks_used)
        write_inodes(f, layout)
        write_file_blocks(f, layout)
        write_free_blocks(f, nblocks_used)

if __name__ == '__main__':
    main()
