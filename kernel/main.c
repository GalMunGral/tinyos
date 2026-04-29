#include "uart.h"
#include "kprintf.h"
#include "trap.h"
#include "mm.h"
#include "vm.h"

void kmain(void) {
    uart_init();
    kprintf("uart ok\n");
    trap_init();
    kprintf("trap ok\n");
    mm_init();
    kprintf("vm: sv39 on, kernel at 0x%x\n", KERNEL_OFFSET);

    for (;;)
        __asm__ volatile("wfi");
}