#pragma once

#include "syscall.h"

static inline void putchar(int c) {
    register long a7 __asm__("a7") = SYSCALL_PUTCHAR;
    register long a0 __asm__("a0") = c;
    __asm__ volatile("ecall" : "+r"(a0) : "r"(a7));
}

static inline void sleep(long ms) {
    register long a7 __asm__("a7") = SYSCALL_SLEEP;
    register long a0 __asm__("a0") = ms;
    __asm__ volatile("ecall" : "+r"(a0) : "r"(a7));
}

static inline void puts(const char *s) {
    while (*s) putchar(*s++);
}
