#pragma once

#include "types.h"

// Matches the save/restore order in trap.S exactly — offsets are 8 * field index.
struct trapframe {
    uint64_t epc;       // 0   sepc — PC that was interrupted
    uint64_t ra;        // 8
    uint64_t sp;        // 16
    uint64_t gp;        // 24
    uint64_t tp;        // 32
    uint64_t t0;        // 40
    uint64_t t1;        // 48
    uint64_t t2;        // 56
    uint64_t s0;        // 64
    uint64_t s1;        // 72
    uint64_t a0;        // 80
    uint64_t a1;        // 88
    uint64_t a2;        // 96
    uint64_t a3;        // 104
    uint64_t a4;        // 112
    uint64_t a5;        // 120
    uint64_t a6;        // 128
    uint64_t a7;        // 136
    uint64_t s2;        // 144
    uint64_t s3;        // 152
    uint64_t s4;        // 160
    uint64_t s5;        // 168
    uint64_t s6;        // 176
    uint64_t s7;        // 184
    uint64_t s8;        // 192
    uint64_t s9;        // 200
    uint64_t s10;       // 208
    uint64_t s11;       // 216
    uint64_t t3;        // 224
    uint64_t t4;        // 232
    uint64_t t5;        // 240
    uint64_t t6;        // 248
};                      // 256 bytes total

void trap_init(void);
void trap_handler(struct trapframe *tf);
void syscall_dispatch(struct trapframe *tf);
void proc_entry(void);
void trap_return(void);