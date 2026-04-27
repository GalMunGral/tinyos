.section .text.entry
.global _start

_start:
    # OpenSBI passes hart ID in a0; only hart 0 proceeds
    bnez    a0, .halt

    la      sp, _stack_top

    # clear BSS
    la      t0, __bss_start
    la      t1, __bss_end
1:  bgeu    t0, t1, 2f
    sd      zero, 0(t0)
    addi    t0, t0, 8
    j       1b

2:  call    kmain

.halt:
    wfi
    j       .halt

.section .bss
.align 4
.space 4096
_stack_top:
