# TinyOS

```sh
cmake -B build && cmake --build build
cmake --build build --target qemu                     # run on QEMU
path/to/tinyvm kernel build/kernel.elf build/disk.img # or run on tinyvm
```

## Rhetorical Design

### Purpose

For those familiar with event loops but not operating systems, a kernel is less alien than it appears. At its core, it is an infinite loop: each iteration picks a process, restores its register state — PC, SP, and the rest — and runs it until a trap fires. The trap, whether a syscall, a timer interrupt, or an exception, returns control to the kernel, which saves the register state, does its work, and loops again. The difference between cooperative and preemptive multitasking reduces to how that return is triggered: a voluntary yield or a timer interrupt. The loop is the same.

### Strategy

The kernel implements the irreducible minimum: trap handling, context switching, process scheduling, virtual memory, system calls, ELF loading, and a basic filesystem. Policy is kept deliberately naive — round-robin scheduling, a free-list allocator, a fixed-size process table — to keep the mechanism visible. The kernel targets RISC-V and runs directly on [tinyvm](https://github.com/GalMunGral/tinyvm), grounding the two projects in a single concrete stack.