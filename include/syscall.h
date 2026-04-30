#pragma once

#include "trap.h"

#define SYSCALL_PUTCHAR 1
#define SYSCALL_SLEEP   2

void syscall_dispatch(struct trapframe *tf);