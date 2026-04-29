#pragma once

#include "types.h"
#include "vm.h"
#include "trap.h"

#define NPROC 8

struct switch_context {
    uint64_t ra, sp;
    uint64_t s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
};

enum proc_state { RUNNABLE, EXITED };

struct proc {
    enum proc_state       state;
    pagetable_t           pagetable;
    struct switch_context ctx;
};

static inline void *kstack_top(struct proc *p) {
    return (char *)p + 4096 - sizeof(struct trapframe);
}

extern struct proc *current_proc;

void         context_switch(struct switch_context *old, struct switch_context *new);
struct proc *proc_alloc(void (*fn)(void));
void         proc_exit(void);
void         yield(void);
void         scheduler(void);
