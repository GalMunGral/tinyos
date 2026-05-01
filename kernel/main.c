#include "uart.h"
#include "kprintf.h"
#include "trap.h"
#include "mem.h"
#include "vm.h"
#include "proc.h"
#include "virtio_blk.h"
#include "fs.h"

void kmain(void) {
    uart_init();
    kprintf("uart ok\n");
    trap_init();
    kprintf("trap ok\n");
    mm_init();
    kprintf("mem ok\n");
    proc_init();
    kprintf("proc ok\n");
    virtio_blk_init();
    kprintf("blk ok\n");
    fs_init();
    kprintf("fs ok\n");
    for (uint64_t c = 0x20; c <= 0x7E; c++)
        proc_alloc("hello", c);
    scheduler();
}