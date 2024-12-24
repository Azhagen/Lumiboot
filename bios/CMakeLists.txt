cmake_minimum_required(VERSION 3.20)

set(PROJECT "lumiboot")
add_executable(${PROJECT})

set(LINKER_SCRIPT "linker/ia16-script.ld")
add_custom_target(LINKER DEPENDS ${LINKER_SCRIPT})
add_dependencies(${PROJECT} LINKER)

set(ENABLE_EXPORTS TRUE)

target_sources(${PROJECT}
    PRIVATE
    "system/reset.S"
    "system/entry.S"
    "system/start.c"
    "system/interrupt.c"
    "system/memory.c"
    "system/oprom.c"
    "system/pmode.S"
    "system/wait.c"
    "system/data.c"
    "system/system.c"
    "system/block.c"
    
    "services/interface/video.c"
    "services/interface/disk.c"
    "services/interface/boot.c"
    "services/interface/system.c"
    "services/interface/keyboard.c"

    "services/interrupts/interrupt_02.S"
    "services/interrupts/interrupt_05.S"
    "services/interrupts/interrupt_08.S"
    "services/interrupts/interrupt_09.S"
    "services/interrupts/interrupt_0E.S"
    "services/interrupts/interrupt_10.S"
    "services/interrupts/interrupt_11.S"
    "services/interrupts/interrupt_12.S"
    "services/interrupts/interrupt_13.S"
    "services/interrupts/interrupt_14.S"
    "services/interrupts/interrupt_15.S"
    "services/interrupts/interrupt_16.S"
    "services/interrupts/interrupt_17.S"
    "services/interrupts/interrupt_18.S"
    "services/interrupts/interrupt_19.S"
    "services/interrupts/interrupt_1A.S"
    "services/interrupts/interrupt_70.S"
    "services/interrupts/interrupt_XX.S"

    "services/boot.c"
    "services/boot.S"
    "services/printer.c"
    "services/keyboard.c"
    "services/serial.c"
    "services/time.c"
    "services/video.c"
    "services/system.c"
    "services/block.c"
    "services/print.c"

    "drivers/chipset/pit.c"
    "drivers/chipset/pic.c"
    "drivers/chipset/dma.c"
    "drivers/hid/kbd.c"
    "drivers/hid/kbd_at.c"
    "drivers/disk/fdc.c"
    "drivers/disk/ata.c"
    "drivers/disk/atapi.c"
    "drivers/misc/uart.c"
    "drivers/misc/joy.c"
    "drivers/misc/lpt.c"
    "drivers/misc/i8042.c"
    "drivers/misc/cmos.c"
    "drivers/audio/spk.c"

    "debug/debug.c"
    "debug/gdbstub.c"
    "debug/interrupt_0Bh.S"
    "debug/interrupt_dbg.S"

    "data/video.c"
    "data/keycode.c"
    "data/charset.c"
    "data/logo.c"
    "data/vtable.c"
    "data/floppy.c"
    "data/system.c"

    "runtime/ubsan.c"
    "runtime/string.c"
    "runtime/ssp.c"
    "runtime/stdio.c"
    "runtime/print.c"

    "setup/tui.c"
    "setup/setup.c"
)

target_include_directories(${PROJECT}
    PUBLIC
    "include"
    PRIVATE
    "src/include"
)

target_compile_definitions(${PROJECT}
    PRIVATE
    "MACHINE_TYPE=${MACHINE_TYPE}"
    "CHIPSET_MODEL=${CHIPSET_MODEL}"
    "DEBUG"
)

target_compile_options(${PROJECT}
    PRIVATE
    "-pipe"

    "-mcmodel=small"
    "-mprotected-mode"
    "-msegelf"
    "-mno-callee-assume-ss-data-segment"
    "-march=i80286"

    "-ffunction-sections"
    "-fdata-sections"

    "-fipa-pta"
    
    "-Wall"
    "-Wextra"
    "-Wconversion"
    "-Warray-bounds"
    "-Wold-style-declaration"
    "-Wold-style-definition"
    "-Wmissing-parameter-type"
    "-Wmissing-field-initializers"

    "-ffreestanding"
    "-funsigned-char"
    "-fexec-charset=cp437"
)

target_link_options(${PROJECT}
    PRIVATE
    "-pipe"
    "-nodefaultlibs"
    "-nostartfiles"
    "-nostdlib"
    "-static"

    "LINKER:--gc-sections"
    "-T${CMAKE_CURRENT_SOURCE_DIR}/${LINKER_SCRIPT}"
    "LINKER:--Map=${CMAKE_BINARY_DIR}/firmware.map"
    "LINKER:-melf_i386"
)

target_link_libraries(${PROJECT}
    PRIVATE
    "gcc"
)

set_target_properties(${PROJECT} PROPERTIES
    SUFFIX  ".elf"
)

add_custom_command(TARGET ${PROJECT} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} ARGS -O binary -D ${PROJECT}.elf ${PROJECT}.bin)

add_custom_command(TARGET ${PROJECT} POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} ARGS -O binary -i 2 -b 0 -D --input-target=binary ${PROJECT}.bin ${PROJECT}_A.bin
    COMMAND ${CMAKE_OBJCOPY} ARGS -O binary -i 2 -b 1 -D --input-target=binary ${PROJECT}.bin ${PROJECT}_B.bin)

add_custom_command(TARGET ${PROJECT} POST_BUILD
    COMMAND cat ARGS ${PROJECT}_A.bin ${PROJECT}_B.bin > ${PROJECT}_oddeven.bin)