#include "proc.h"
#include "mem.h"
#include "vm.h"
#include "util.h"
#include "csr.h"
#include "kprintf.h"
#include "elf.h"

struct proc *procs[NPROC];
struct proc *current_proc;

static struct switch_context scheduler_ctx;
static uint64_t default_satp;

extern pte_t default_pagetable[];

void proc_init(void) {
    default_satp = pt2satp(default_pagetable);
}

static pte_t *setup_pagetable(void) {
    pte_t *pt = alloc_pagetable();
    pt[256] = default_pagetable[256];   // devices: VA 0xFFFFFFC000000000
    pt[258] = default_pagetable[258];   // kernel:  VA 0xFFFFFFC080000000
    return pt;
}

static uint64_t alloc_stack(pte_t *pt) {
    void *stack = alloc_page();
    if (!stack) kpanic("alloc_stack: out of memory\n");
    map_page(pt, USER_STACK_VA, kva2pa(stack), PTE_R | PTE_W | PTE_U);
    return USER_STACK_VA + PAGE_SIZE;
}

struct proc *proc_alloc(const char *name, uint64_t arg) {
    void *page = alloc_page();
    if (!page) kpanic("proc_alloc: out of memory\n");

    struct proc *p = (struct proc *)page;
    p->state     = RUNNABLE;
    p->ctx.ra    = (uint64_t)proc_entry;
    p->ctx.sp    = (uint64_t)kstack_top(p) - sizeof(struct trapframe);

    p->pagetable  = setup_pagetable();
    uint64_t entry = elf_load(p, name);
    uint64_t user_sp = alloc_stack(p->pagetable);
    p->satp = pt2satp(p->pagetable);

    struct trapframe *tf = (struct trapframe *)p->ctx.sp;
    tf->epc = entry;
    tf->sp  = user_sp;
    tf->a1  = arg;

    for (int i = 0; i < NPROC; i++) {
        if (!procs[i]) {
            tf->a0  = i;
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
    struct proc *p = procs[i];
    procs[i] = NULL;
    free_pagetable(p->pagetable);
    free_page(p);
}

void scheduler(void) {
    for (;;) {
        for (int i = 0; i < NPROC; i++) {
            if (!procs[i]) continue;
            if (procs[i]->state == SLEEPING && r_time() >= procs[i]->wake_time)
                procs[i]->state = RUNNABLE;
            if (procs[i]->state != RUNNABLE) continue;
            current_proc = procs[i];
            w_satp(current_proc->satp);
            sfence_vma();
            context_switch(&scheduler_ctx, &current_proc->ctx);
            w_satp(default_satp);
            sfence_vma();
            if (current_proc->state == EXITED)
                proc_free(i);
            current_proc = NULL;
        }
    }
}
