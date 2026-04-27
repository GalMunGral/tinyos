#include "uart.h"

#define UART_BASE 0x10000000UL

#define RBR 0  // receive buffer   (DLAB=0, read)
#define THR 0  // transmit holding (DLAB=0, write)
#define IER 1  // interrupt enable
#define FCR 2  // FIFO control
#define LCR 3  // line control
#define LSR 5  // line status

#define LCR_8N1  0x03  // 8 data bits, no parity, 1 stop bit
#define LCR_DLAB 0x80  // divisor latch access
#define FCR_FIFO 0x01  // enable FIFO
#define LSR_THRE 0x20  // transmit holding register empty
#define LSR_DR   0x01  // data ready

static volatile unsigned char *const uart = (volatile unsigned char *)UART_BASE;

void uart_init(void) {
    uart[IER] = 0x00;       // disable interrupts
    uart[LCR] = LCR_DLAB;  // enable divisor latch
    uart[RBR] = 0x01;       // divisor LSB (115200 baud @ 1.8432 MHz)
    uart[IER] = 0x00;       // divisor MSB
    uart[LCR] = LCR_8N1;   // 8N1, clear DLAB
    uart[FCR] = FCR_FIFO;  // enable FIFO
}

void uart_putc(char c) {
    while (!(uart[LSR] & LSR_THRE))
        ;
    uart[THR] = c;
}

void uart_puts(const char *s) {
    while (*s)
        uart_putc(*s++);
}

int uart_getc(void) {
    if (uart[LSR] & LSR_DR)
        return uart[RBR];
    return -1;
}