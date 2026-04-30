#pragma once

#include "types.h"

#define TIMER_TICKS_PER_MS 10000UL  // QEMU virt mtime runs at 10MHz

static inline uint64_t r_scause(void) {
    uint64_t x;
    __asm__ volatile("csrr %0, scause" : "=r"(x));
    return x;
}

static inline uint64_t r_stval(void) {
    uint64_t x;
    __asm__ volatile("csrr %0, stval" : "=r"(x));
    return x;
}

static inline uint64_t r_time(void) {
    uint64_t x;
    __asm__ volatile("csrr %0, time" : "=r"(x));
    return x;
}
