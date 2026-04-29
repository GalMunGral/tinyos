#include "proc.h"
#include "mm.h"
#include "kprintf.h"

struct proc *procs[NPROC];
struct proc *current_proc;

static struct switch_context scheduler_ctx;

struct proc *proc_alloc(void (*fn)(void)) {
    void *page = alloc_page();
    if (!page) kpanic("proc_alloc: out of memory\n");

    struct proc *p = (struct proc *)page;
    p->state     = RUNNABLE;
    p->pagetable = 0;
    p->ctx.ra    = (uint64_t)fn;
    p->ctx.sp    = (uint64_t)kstack_top(p);

    for (int i = 0; i < NPROC; i++) {
        if (!procs[i]) {
            procs[i] = p;
            return p;
        }
    }
    kpanic("proc_alloc: no free slot\n");
    return NULL;
}

void proc_exit(void) {
    current_proc->state = EXITED;
    yield();
}

void yield(void) {
    context_switch(&current_proc->ctx, &scheduler_ctx);
}

static void proc_free(int i) {
    free_page(procs[i]);
    procs[i] = NULL;
}

void scheduler(void) {
    for (;;) {
        for (int i = 0; i < NPROC; i++) {
            if (!procs[i] || procs[i]->state != RUNNABLE) continue;
            current_proc = procs[i];
            context_switch(&scheduler_ctx, &current_proc->ctx);
            if (current_proc->state == EXITED)
                proc_free(i);
            current_proc = NULL;
        }
    }
}
