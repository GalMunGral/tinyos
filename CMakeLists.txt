cmake_minimum_required(VERSION 3.20)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_TOOLCHAIN_FILE ${CMAKE_SOURCE_DIR}/cmake/riscv64-toolchain.cmake)

project(tinyos C ASM)

set(CMAKE_C_STANDARD 11)

set(ARCH_FLAGS -march=rv64gc -mabi=lp64d -mcmodel=medany)

set(KERNEL_COMPILE_FLAGS
    -ffreestanding
    -fno-stack-protector
    -fno-pie
    -fno-pic
    -nostdlib
    -nostdinc
    -O0
    -g
    -Wall
    -Wextra
)

set(LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/kernel/kernel.ld)

file(GLOB_RECURSE KERNEL_SOURCES
    CONFIGURE_DEPENDS
    kernel/*.c
    kernel/*.S
)

add_executable(kernel.elf ${KERNEL_SOURCES})

target_compile_options(kernel.elf PRIVATE ${ARCH_FLAGS} ${KERNEL_COMPILE_FLAGS})
target_include_directories(kernel.elf PRIVATE include)
target_link_options(kernel.elf PRIVATE
    ${ARCH_FLAGS}
    -nostdlib
    -T ${LINKER_SCRIPT}
    -Wl,--build-id=none
)

# QEMU
set(QEMU_BASE
    qemu-system-riscv64
    -machine virt
    -bios default
    -kernel $<TARGET_FILE:kernel.elf>
    -nographic
    -m 128M
)

add_custom_target(qemu
    COMMAND ${QEMU_BASE}
    DEPENDS kernel.elf
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    USES_TERMINAL
)

add_custom_target(qemu-gdb
    COMMAND ${QEMU_BASE} -S -gdb tcp::1234
    DEPENDS kernel.elf
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    USES_TERMINAL
)
