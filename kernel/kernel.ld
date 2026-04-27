OUTPUT_ARCH(riscv)
ENTRY(_start)

KERNEL_BASE = 0x80200000;

SECTIONS {
    . = KERNEL_BASE;

    .text : {
        *(.text.entry)      /* entry.S must be first */
        *(.text .text.*)
    }

    . = ALIGN(4096);

    .rodata : {
        *(.rodata .rodata.*)
    }

    . = ALIGN(4096);

    .data : {
        *(.data .data.*)
    }

    . = ALIGN(4096);

    .bss : {
        __bss_start = .;
        *(.bss .bss.*)
        *(.sbss .sbss.*)
        __bss_end = .;
    }

    . = ALIGN(4096);
    __kernel_end = .;
}
