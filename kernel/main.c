#include "uart.h"
#include "kprintf.h"
#include "trap.h"
#include "mm.h"
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
    kprintf("mm ok\n");
    proc_init();
    virtio_blk_init();
    fs_init();
    for (uint64_t c = 0x20; c <= 0x7E; c++)
        proc_alloc("hello", c);
    scheduler();
}