.equ KERNEL_OFFSET, 0xFFFFFFC000000000

.section .text.entry
.global _start
.global _start_virtual

_start:
    bnez    a0, .halt               # only hart 0 boots

    # Clear BSS (paging off; la gives PA via auipc + small offset)
    la      t0, __bss_start
    la      t1, __bss_end
1:  bgeu    t0, t1, 2f
    sd      zero, 0(t0)
    addi    t0, t0, 8
    j       1b

2:  # Build default page table (BSS is now zeroed; la gives PA)
    la      t0, default_pagetable

    # root[2]:   VA 0x80000000         -> PA 0x80000000  (identity, R/W/X; needed until jump to high VA)
    # root[258]: VA 0xFFFFFFC080000000 -> PA 0x80000000  (kernel,   R/W/X)
    #   PPN = (0x80000000 >> 12) << 10 = 0x80000 << 10 = 0x20000000
    #   flags: D=1 A=1 G=0 U=0 X=1 W=1 R=1 V=1 = 0xCF
    li      t1, 0x00000000200000CF
    sd      t1, (2*8)(t0)           # offset 16: fits in 12-bit imm
    li      t2, (258*8)             # offset 2064: too large for 12-bit imm, use register
    add     t2, t0, t2
    sd      t1, 0(t2)

    # root[256]: VA 0xFFFFFFC000000000 -> PA 0x00000000  (devices, R/W)
    #   PPN = 0x000 << 10 = 0x00000000
    #   flags: D=1 A=1 G=0 U=0 X=0 W=1 R=1 V=1 = 0xC7
    li      t1, 0x00000000000000C7
    li      t2, (256*8)             # offset 2048: too large for 12-bit imm, use register
    add     t2, t0, t2
    sd      t1, 0(t2)

    # Enable Sv39: satp = (8<<60) | PPN-of-default_pagetable
    srli    t1, t0, 12
    li      t2, 8
    slli    t2, t2, 60
    or      t1, t1, t2
    csrw    satp, t1
    sfence.vma zero, zero

    # la gives PA here (PC is still identity-mapped); add KERNEL_OFFSET for VA
    la      t0, _start_virtual
    li      t1, KERNEL_OFFSET
    add     t0, t0, t1
    jalr    zero, t0, 0             # absolute jump to high VA

_start_virtual:
    la      sp, _stack_top          # la now gives VA
    call    kmain

.halt:
    wfi
    j       .halt

.section .bss
.align 12                           # 4 KB page-aligned
.global default_pagetable
default_pagetable:
.space 4096                         # 512 x 8-byte PTEs

.align 4                            # 16-byte stack alignment (RISC-V ABI)
.space 4096
_stack_top:
