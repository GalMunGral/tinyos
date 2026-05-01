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

set_target_properties(kernel.elf PROPERTIES LINK_DEPENDS ${LINKER_SCRIPT})

# User programs
set(USER_LINKER_SCRIPT ${CMAKE_SOURCE_DIR}/user/user.ld)
set(USER_COMPILE_FLAGS
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

file(GLOB USER_SOURCES CONFIGURE_DEPENDS user/*.c)

set(USER_TARGETS)
set(USER_BINS)
foreach(src ${USER_SOURCES})
    get_filename_component(name ${src} NAME_WE)
    add_executable(${name} ${src} user/crt0.S)
    target_compile_options(${name} PRIVATE ${ARCH_FLAGS} ${USER_COMPILE_FLAGS})
    target_include_directories(${name} PRIVATE include user)
    target_link_options(${name} PRIVATE
        ${ARCH_FLAGS}
        -nostdlib
        -T ${USER_LINKER_SCRIPT}
        -Wl,--build-id=none
    )
    set_target_properties(${name} PROPERTIES LINK_DEPENDS ${USER_LINKER_SCRIPT})
    list(APPEND USER_TARGETS ${name})
    list(APPEND USER_BINS $<TARGET_FILE:${name}>)
endforeach()

# Disk image
set(MKFS_PY   ${CMAKE_SOURCE_DIR}/tools/mkfs.py)
set(DISK_IMG  ${CMAKE_BINARY_DIR}/disk.img)

add_custom_command(
    OUTPUT  ${DISK_IMG}
    COMMAND python3 ${MKFS_PY} ${DISK_IMG} ${USER_BINS}
    DEPENDS ${MKFS_PY} ${USER_TARGETS}
)

# QEMU
set(QEMU_BASE
    qemu-system-riscv64
    -machine virt
    -bios default
    -kernel $<TARGET_FILE:kernel.elf>
    -nographic
    -m 128M
    -drive file=${DISK_IMG},format=raw,if=none,id=hd0
    -device virtio-blk-device,drive=hd0,bus=virtio-mmio-bus.0
    -global virtio-mmio.force-legacy=false
)

add_custom_target(qemu
    COMMAND ${QEMU_BASE}
    DEPENDS kernel.elf ${DISK_IMG}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    USES_TERMINAL
)

add_custom_target(qemu-gdb
    COMMAND ${QEMU_BASE} -S -gdb tcp::1234
    DEPENDS kernel.elf ${DISK_IMG}
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    USES_TERMINAL
)
