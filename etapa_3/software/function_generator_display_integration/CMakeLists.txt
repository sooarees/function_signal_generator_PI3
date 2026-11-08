cmake_minimum_required(VERSION 3.20)
project(function_generator_display_integration C ASM)

set(FREERTOS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Middlewares/FreeRTOS-Kernel)
set(CMSIS_CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Core)
set(CMSIS_DEVICE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Drivers/CMSIS/Device/ST/STM32F4xx)

add_executable(function_generator_display_integration
    Startup/startup_stm32f439zitx.s
    BSP/system_clock.c
    Sources/main.c
    Sources/memory.c
    Control/signal_control.c
    Drivers/analog_self_test.c
    Drivers/diagnostic_uart.c
    Drivers/display.c
    Drivers/touch.c
    Drivers/waveform_generator.c
    Tasks/signal_control_task.c
    Tasks/serial_control_task.c
    Tasks/ui_task.c
    UI/ui.c
    ${CMSIS_DEVICE_DIR}/Source/Templates/system_stm32f4xx.c
    ${FREERTOS_DIR}/tasks.c
    ${FREERTOS_DIR}/queue.c
    ${FREERTOS_DIR}/list.c
    ${FREERTOS_DIR}/portable/GCC/ARM_CM4F/port.c
    ${FREERTOS_DIR}/portable/MemMang/heap_4.c
)

set_target_properties(function_generator_display_integration PROPERTIES SUFFIX ".elf")

target_compile_definitions(function_generator_display_integration PRIVATE
    STM32F439xx
    HSE_VALUE=8000000U
)

target_include_directories(function_generator_display_integration PRIVATE
    BSP
    Control
    Drivers
    Includes
    Sources
    Tasks
    UI
    ${CMSIS_CORE_DIR}/Include
    ${CMSIS_DEVICE_DIR}/Include
    ${FREERTOS_DIR}/include
    ${FREERTOS_DIR}/portable/GCC/ARM_CM4F
)

target_compile_options(function_generator_display_integration PRIVATE
    -mcpu=cortex-m4
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
    -mthumb
    -ffreestanding
    -fno-builtin
    -ffunction-sections
    -fdata-sections
    -Wall
    -Werror
    $<$<COMPILE_LANGUAGE:C>:-std=gnu11>
)

target_link_options(function_generator_display_integration PRIVATE
    -mcpu=cortex-m4
    -mfpu=fpv4-sp-d16
    -mfloat-abi=hard
    -mthumb
    -nostdlib
    -nostartfiles
    -T${CMAKE_CURRENT_SOURCE_DIR}/STM32F439ZITX_FLASH.ld
    -Wl,-Map=function_generator_display_integration.map
    -Wl,--gc-sections
    -static
)

target_link_libraries(function_generator_display_integration PRIVATE gcc)

add_custom_command(TARGET function_generator_display_integration POST_BUILD
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:function_generator_display_integration>
)
