#include "uart.h"
#include "kprintf.h"
#include "trap.h"
#include "mm.h"
#include "vm.h"
#include "proc.h"

static void proc_a(void) {
    for (;;)
        kprintf("A\n");
}

static void proc_b(void) {
    for (;;)
        kprintf("B\n");
}

void kmain(void) {
    uart_init();
    kprintf("uart ok\n");
    trap_init();
    kprintf("trap ok\n");
    mm_init();
    kprintf("mm ok\n");
    proc_init();
    proc_alloc(proc_a);
    proc_alloc(proc_b);
    scheduler();
}