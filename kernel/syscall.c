#include "trap.h"
#include "syscall.h"
#include "proc.h"
#include "csr.h"
#include "uart.h"
#include "kprintf.h"

static void sys_write(struct trapframe *tf) {
    const char *buf = (const char *)tf->a0;
    uint32_t len = (uint32_t)tf->a1;
    for (uint32_t i = 0; i < len; i++)
        uart_putc(buf[i]);
}

static void sys_printf(struct trapframe *tf) {
    kprintf((const char *)tf->a0, tf->a1, tf->a2, tf->a3, tf->a4);
}

static void sys_sleep(struct trapframe *tf) {
    current_proc->wake_time = r_time() + tf->a0 * TIMER_TICKS_PER_MS;
    current_proc->state     = SLEEPING;
    yield();
}

void syscall_dispatch(struct trapframe *tf) {
    tf->epc += 4;
    switch (tf->a7) {
    case SYSCALL_WRITE:
        sys_write(tf);
        break;
    case SYSCALL_PRINTF:
        sys_printf(tf);
        break;
    case SYSCALL_SLEEP:
        sys_sleep(tf);
        break;
    case SYSCALL_EXIT:
        proc_exit();
        break;
    default:
        kpanic("unknown syscall: a7=%x\n", tf->a7);
    }
}
