OUTPUT_ARCH(riscv)
ENTRY(_start)

KERNEL_OFFSET  = 0xFFFFFFC000000000;
KERNEL_BASE_PA = 0x80200000;
KERNEL_BASE_VA = KERNEL_OFFSET + KERNEL_BASE_PA;

/* LMA(section) = ADDR(section) - KERNEL_BASE_VA + KERNEL_BASE_PA
 * Avoids overflow: compute small offset-within-kernel first, then add physical base.
 * AT(.) doesn't work because . = 0 at parse time; ADDR() is resolved at link time. */

SECTIONS {
    . = KERNEL_BASE_VA;

    .text : AT(KERNEL_BASE_PA) {
        *(.text.entry)
        *(.text .text.*)
    }

    . = ALIGN(4096);
    .rodata : AT(ADDR(.rodata) - KERNEL_BASE_VA + KERNEL_BASE_PA) {
        *(.rodata .rodata.*)
    }

    . = ALIGN(4096);
    .data : AT(ADDR(.data) - KERNEL_BASE_VA + KERNEL_BASE_PA) {
        *(.data .data.*)
    }

    . = ALIGN(4096);
    .bss (NOLOAD) : AT(ADDR(.bss) - KERNEL_BASE_VA + KERNEL_BASE_PA) {
        __bss_start = .;
        *(.bss .bss.*)
        *(.sbss .sbss.*)
        __bss_end = .;
    }

    . = ALIGN(4096);
    __kernel_end = .;
}