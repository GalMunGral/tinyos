#include "syscall.h"

.section .text.entry
.global _start
_start:
    call main
    li a7, SYSCALL_EXIT
    ecall
1:  j 1b
