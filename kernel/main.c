#include "uart.h"
#include "kprintf.h"
#include "trap.h"

void kmain(void) {
    uart_init();
    kprintf("uart ok\n");
    trap_init();
    kprintf("trap ok\n");

    for (;;)
        __asm__ volatile("wfi");
}