#include "syscall.h"
#include "trap.h"
#include "kprintf.h"

static void sys_putchar(struct trapframe *tf) {
    kprintf("%c", (char)tf->a0);
}

void syscall_dispatch(struct trapframe *tf) {
    tf->epc += 4;
    switch (tf->a7) {
    case SYSCALL_PUTCHAR:
        sys_putchar(tf);
        break;
    default:
        kpanic("unknown syscall: a7=%x\n", tf->a7);
    }
}
