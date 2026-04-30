.section .text
.global kernel_trap_entry
.global user_trap_entry
.global proc_entry
.global trap_return
.align 4

# Trap from kernel mode: sp is already on the kstack; allocate trapframe below it.
kernel_trap_entry:
    addi    sp, sp, -256

    # save t0 first so we can use it as scratch; save sepc and original sp
    sd      t0,  40(sp)
    csrr    t0, sepc
    sd      t0,   0(sp)
    addi    t0, sp, 256
    sd      t0,  16(sp)

    # save remaining GPRs
    sd      ra,   8(sp)
    sd      gp,  24(sp)
    sd      tp,  32(sp)
    sd      t1,  48(sp)
    sd      t2,  56(sp)
    sd      s0,  64(sp)
    sd      s1,  72(sp)
    sd      a0,  80(sp)
    sd      a1,  88(sp)
    sd      a2,  96(sp)
    sd      a3, 104(sp)
    sd      a4, 112(sp)
    sd      a5, 120(sp)
    sd      a6, 128(sp)
    sd      a7, 136(sp)
    sd      s2, 144(sp)
    sd      s3, 152(sp)
    sd      s4, 160(sp)
    sd      s5, 168(sp)
    sd      s6, 176(sp)
    sd      s7, 184(sp)
    sd      s8, 192(sp)
    sd      s9, 200(sp)
    sd      s10, 208(sp)
    sd      s11, 216(sp)
    sd      t3, 224(sp)
    sd      t4, 232(sp)
    sd      t5, 240(sp)
    sd      t6, 248(sp)

    mv      a0, sp
    call    trap_handler

    # restore sepc and GPRs; sp and t0 last
    ld      t0,   0(sp)
    csrw    sepc, t0

    ld      ra,   8(sp)
    ld      gp,  24(sp)
    ld      tp,  32(sp)
    ld      t1,  48(sp)
    ld      t2,  56(sp)
    ld      s0,  64(sp)
    ld      s1,  72(sp)
    ld      a0,  80(sp)
    ld      a1,  88(sp)
    ld      a2,  96(sp)
    ld      a3, 104(sp)
    ld      a4, 112(sp)
    ld      a5, 120(sp)
    ld      a6, 128(sp)
    ld      a7, 136(sp)
    ld      s2, 144(sp)
    ld      s3, 152(sp)
    ld      s4, 160(sp)
    ld      s5, 168(sp)
    ld      s6, 176(sp)
    ld      s7, 184(sp)
    ld      s8, 192(sp)
    ld      s9, 200(sp)
    ld      s10, 208(sp)
    ld      s11, 216(sp)
    ld      t3, 224(sp)
    ld      t4, 232(sp)
    ld      t5, 240(sp)
    ld      t6, 248(sp)
    ld      t0,  40(sp)
    ld      sp,  16(sp)

    sret

# Trap from user mode: sscratch = kstack_top; swap sp, then allocate trapframe.
user_trap_entry:
    csrrw   sp, sscratch, sp        # sp = kstack_top, sscratch = user sp
    addi    sp, sp, -256            # allocate trapframe (mirrors kernel_trap_entry)

    # save t0 first; recover user sp and sepc via t0
    sd      t0,  40(sp)
    csrr    t0, sscratch
    sd      t0,  16(sp)             # save user sp
    csrr    t0, sepc
    sd      t0,   0(sp)

    # save remaining GPRs
    sd      ra,   8(sp)
    sd      gp,  24(sp)
    sd      tp,  32(sp)
    sd      t1,  48(sp)
    sd      t2,  56(sp)
    sd      s0,  64(sp)
    sd      s1,  72(sp)
    sd      a0,  80(sp)
    sd      a1,  88(sp)
    sd      a2,  96(sp)
    sd      a3, 104(sp)
    sd      a4, 112(sp)
    sd      a5, 120(sp)
    sd      a6, 128(sp)
    sd      a7, 136(sp)
    sd      s2, 144(sp)
    sd      s3, 152(sp)
    sd      s4, 160(sp)
    sd      s5, 168(sp)
    sd      s6, 176(sp)
    sd      s7, 184(sp)
    sd      s8, 192(sp)
    sd      s9, 200(sp)
    sd      s10, 208(sp)
    sd      s11, 216(sp)
    sd      t3, 224(sp)
    sd      t4, 232(sp)
    sd      t5, 240(sp)
    sd      t6, 248(sp)

    # mark kernel mode: sscratch=0, stvec=kernel_trap_entry
    csrw    sscratch, zero
    la      t0, kernel_trap_entry
    csrw    stvec, t0

    mv      a0, sp
    call    trap_handler
    j       trap_return             # always return to user via trap_return

# First entry into a new user process: sstatus not set up by hardware, fix it before
# falling through to trap_return.
proc_entry:
    li      t0, (1 << 8)
    csrc    sstatus, t0             # SPP=0: sret returns to user mode
    li      t0, (1 << 5)
    csrs    sstatus, t0             # SPIE=1: interrupts enabled after sret
    # fall through

# Return to user mode after a trap: sp = trapframe base.
trap_return:
    ld      t0,   0(sp)
    csrw    sepc, t0                # restore user PC

    # arm next user trap: sscratch=kstack_top, stvec=user_trap_entry
    addi    t0, sp, 256
    csrw    sscratch, t0            # sscratch = kstack_top
    la      t0, user_trap_entry
    csrw    stvec, t0

    # restore GPRs; sp last since it holds the trapframe pointer until the end
    ld      ra,   8(sp)
    ld      gp,  24(sp)
    ld      tp,  32(sp)
    ld      t0,  40(sp)
    ld      t1,  48(sp)
    ld      t2,  56(sp)
    ld      s0,  64(sp)
    ld      s1,  72(sp)
    ld      a0,  80(sp)
    ld      a1,  88(sp)
    ld      a2,  96(sp)
    ld      a3, 104(sp)
    ld      a4, 112(sp)
    ld      a5, 120(sp)
    ld      a6, 128(sp)
    ld      a7, 136(sp)
    ld      s2, 144(sp)
    ld      s3, 152(sp)
    ld      s4, 160(sp)
    ld      s5, 168(sp)
    ld      s6, 176(sp)
    ld      s7, 184(sp)
    ld      s8, 192(sp)
    ld      s9, 200(sp)
    ld      s10, 208(sp)
    ld      s11, 216(sp)
    ld      t3, 224(sp)
    ld      t4, 232(sp)
    ld      t5, 240(sp)
    ld      t6, 248(sp)
    ld      sp,  16(sp)             # user sp last

    sret