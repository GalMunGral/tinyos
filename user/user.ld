OUTPUT_ARCH(riscv)
ENTRY(_start)

SECTIONS {
    . = 0x10000;

    .text   : { *(.text.entry) *(.text .text.*) }

    . = ALIGN(4096);
    .rodata : { *(.rodata .rodata.*) }

    . = ALIGN(4096);
    .data   : { *(.data .data.*) }

    . = ALIGN(4096);
    .bss (NOLOAD) : {
        *(.bss .bss.*)
    }
}