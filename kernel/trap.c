#include "trap.h"
#include "proc.h"
#include "syscall.h"
#include "kprintf.h"
#include "csr.h"

#define SCAUSE_INTR   (1UL << 63)
#define CAUSE_TIMER   5
#define CAUSE_ECALL_U 8

#define TIMER_INTERVAL 1000000  // 100ms at 10MHz QEMU clock

static void timer_ack(void) {
    uint64_t t = r_time() + TIMER_INTERVAL;
    register uint64_t a0 __asm__("a0") = t;
    register uint64_t a7 __asm__("a7") = 0;  // SBI_SET_TIMER
    __asm__ volatile("ecall" :: "r"(a0), "r"(a7));
}

static void handle_interrupt(uint64_t code) {
    switch (code) {
    case CAUSE_TIMER:
        timer_ack();
        if (current_proc)
            yield();
        break;
    default:
        kpanic("unknown interrupt: code=%x\n", code);
    }
}

static void handle_exception(struct trapframe *tf, uint64_t code) {
    switch (code) {
    case CAUSE_ECALL_U:
        syscall_dispatch(tf);
        break;
    default:
        kpanic("exception: code=%x stval=%x epc=%x\n", code, r_stval(), tf->epc);
    }
}

void trap_handler(struct trapframe *tf) {
    uint64_t scause = r_scause();
    bool is_intr    = (scause & SCAUSE_INTR) != 0;
    uint64_t code   = scause & ~SCAUSE_INTR;

    if (is_intr)
        handle_interrupt(code);
    else
        handle_exception(tf, code);
}

extern void kernel_trap_entry(void);

void trap_init(void) {
    __asm__ volatile("csrw stvec, %0"  :: "r"(kernel_trap_entry));
    __asm__ volatile("csrs sie, %0"    :: "r"(1 << 5));
    timer_ack();
    __asm__ volatile("csrs sstatus, %0" :: "r"(1 << 1));   // SIE
    __asm__ volatile("csrs sstatus, %0" :: "r"(1 << 18));  // SUM: allow kernel to access user pages
}
