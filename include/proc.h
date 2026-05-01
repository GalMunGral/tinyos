#pragma once

#include "types.h"
#include "mm.h"
#include "vm.h"
#include "trap.h"

#define NPROC         128
#define USER_CODE_VA  0x1000UL   // user code mapped here
#define USER_STACK_VA 0x3000UL   // user stack page; sp starts at top (0x4000)

struct switch_context {
    uint64_t ra, sp;
    uint64_t s0, s1, s2, s3, s4, s5, s6, s7, s8, s9, s10, s11;
};

enum proc_state { RUNNABLE, SLEEPING, EXITED };

struct proc {
    enum proc_state       state;
    uint64_t              satp;
    pte_t                *pagetable;
    struct switch_context ctx;
    uint64_t              wake_time;  // mtime value at which to wake (only valid when SLEEPING)
};

static inline void *kstack_top(struct proc *p) {
    return (char *)p + PAGE_SIZE;
}

extern struct proc *current_proc;

void         context_switch(struct switch_context *old, struct switch_context *new);
void         proc_init(void);
struct proc *proc_alloc(const char *name, uint64_t arg);
void         proc_exit(void);
void         yield(void);
void         scheduler(void);
