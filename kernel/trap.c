#include "trap.h"
#include "proc.h"
#include "kprintf.h"

#define SCAUSE_INTR   (1UL << 63)
#define CAUSE_TIMER   5
#define CAUSE_ECALL_U 8
#define CAUSE_ECALL_S 9

#define TIMER_INTERVAL 1000000  // 100ms at 10MHz QEMU clock

static uint64_t r_scause(void) {
    uint64_t x;
    __asm__ volatile("csrr %0, scause" : "=r"(x));
    return x;
}

static uint64_t r_stval(void) {
    uint64_t x;
    __asm__ volatile("csrr %0, stval" : "=r"(x));
    return x;
}

static uint64_t r_time(void) {
    uint64_t x;
    __asm__ volatile("csrr %0, time" : "=r"(x));
    return x;
}

static void timer_ack(void) {
    uint64_t t = r_time() + TIMER_INTERVAL;
    __asm__ volatile("csrw stimecmp, %0" :: "r"(t));
}

void trap_handler(struct trapframe *tf) {
    uint64_t scause = r_scause();
    uint64_t stval  = r_stval();
    bool is_intr    = (scause & SCAUSE_INTR) != 0;
    uint64_t code   = scause & ~SCAUSE_INTR;

    if (is_intr && code == CAUSE_TIMER) {
        timer_ack();
        if (current_proc)
            yield();
        return;
    }

    kpanic("trap: scause=%x stval=%x epc=%x\n", scause, stval, tf->epc);
}

extern void kernel_trap_entry(void);

void trap_init(void) {
    __asm__ volatile("csrw stvec, %0"  :: "r"(kernel_trap_entry));  // set trap vector
    __asm__ volatile("csrs sie, %0"    :: "r"(1 << 5));      // enable supervisor timer interrupt
    timer_ack();                                              // arm the first timer
    __asm__ volatile("csrs sstatus, %0" :: "r"(1 << 1));     // enable global interrupts (SIE)
}
