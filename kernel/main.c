#include "uart.h"
#include "kprintf.h"

void kmain(void) {
    uart_init();
    kprintf("tinyos booting...\n");

    for (;;)
        __asm__ volatile("wfi");
}