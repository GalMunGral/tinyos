#include "kprintf.h"
#include "uart.h"

static void print_uint(unsigned long n, int base) {
    static const char digits[] = "0123456789abcdef";
    char buf[64];
    int i = 0;
    if (n == 0) {
        uart_putc('0');
        return;
    }
    while (n > 0) {
        buf[i++] = digits[n % base];
        n /= base;
    }
    while (i > 0)
        uart_putc(buf[--i]);
}

static void kvprintf(const char *fmt, __builtin_va_list ap) {
    for (; *fmt; fmt++) {
        if (*fmt != '%') {
            uart_putc(*fmt);
            continue;
        }
        switch (*++fmt) {
        case 'd': {
            long n = __builtin_va_arg(ap, long);
            if (n < 0) { uart_putc('-'); n = -n; }
            print_uint(n, 10);
            break;
        }
        case 'u':
            print_uint(__builtin_va_arg(ap, unsigned long), 10);
            break;
        case 'x':
            print_uint(__builtin_va_arg(ap, unsigned long), 16);
            break;
        case 'p':
            uart_puts("0x");
            print_uint((unsigned long)__builtin_va_arg(ap, void *), 16);
            break;
        case 's': {
            const char *s = __builtin_va_arg(ap, const char *);
            uart_puts(s ? s : "(null)");
            break;
        }
        case 'c':
            uart_putc((char)__builtin_va_arg(ap, int));
            break;
        case '%':
            uart_putc('%');
            break;
        }
    }
}

void kprintf(const char *fmt, ...) {
    __builtin_va_list ap;
    __builtin_va_start(ap, fmt);
    kvprintf(fmt, ap);
    __builtin_va_end(ap);
}

void kpanic(const char *fmt, ...) {
    uart_puts("\nKERNEL PANIC: ");
    __builtin_va_list ap;
    __builtin_va_start(ap, fmt);
    kvprintf(fmt, ap);
    __builtin_va_end(ap);
    uart_puts("\n");
    for (;;)
        __asm__ volatile("wfi");
}
