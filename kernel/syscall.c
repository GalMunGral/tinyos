#include "trap.h"
#include "syscall.h"
#include "proc.h"
#include "csr.h"
#include "kprintf.h"

static void sys_putchar(struct trapframe *tf) {
    kprintf("%c", (char)tf->a0);
}

static void sys_sleep(struct trapframe *tf) {
    current_proc->wake_time = r_time() + tf->a0 * TIMER_TICKS_PER_MS;
    current_proc->state     = SLEEPING;
    yield();
}

void syscall_dispatch(struct trapframe *tf) {
    tf->epc += 4;
    switch (tf->a7) {
    case SYSCALL_PUTCHAR:
        sys_putchar(tf);
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
