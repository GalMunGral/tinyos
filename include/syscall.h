#pragma once

#include "trap.h"

#define SYSCALL_PUTCHAR 1

void syscall_dispatch(struct trapframe *tf);