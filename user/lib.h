#pragma once

#include "syscall.h"

static inline void sleep(long ms) {
    register long a7 __asm__("a7") = SYSCALL_SLEEP;
    register long a0 __asm__("a0") = ms;
    __asm__ volatile("ecall" : "+r"(a0) : "r"(a7));
}

static inline void write(const char *buf, int len) {
    register long a7 __asm__("a7") = SYSCALL_WRITE;
    register long a0 __asm__("a0") = (long)buf;
    register long a1 __asm__("a1") = (long)len;
    __asm__ volatile("ecall" : "+r"(a0) : "r"(a7), "r"(a1));
}

static inline void puts(const char *s) {
    int len = 0;
    while (s[len]) len++;
    write(s, len);
}

static inline void printf(const char *fmt, ...) {
    __builtin_va_list ap;
    __builtin_va_start(ap, fmt);
    long a1 = __builtin_va_arg(ap, long);
    long a2 = __builtin_va_arg(ap, long);
    long a3 = __builtin_va_arg(ap, long);
    long a4 = __builtin_va_arg(ap, long);
    __builtin_va_end(ap);

    register long a7  __asm__("a7") = SYSCALL_PRINTF;
    register long ra0 __asm__("a0") = (long)fmt;
    register long ra1 __asm__("a1") = a1;
    register long ra2 __asm__("a2") = a2;
    register long ra3 __asm__("a3") = a3;
    register long ra4 __asm__("a4") = a4;
    __asm__ volatile("ecall" :: "r"(a7), "r"(ra0), "r"(ra1), "r"(ra2), "r"(ra3), "r"(ra4));
}
