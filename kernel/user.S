.section .text

.align 12
.global user_print_loop
user_print_loop:
    mv      s0, a0          # save character (a0 clobbered by ecall)
1:
    li      a7, 1
    mv      a0, s0
    ecall
    j       1b