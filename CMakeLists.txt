# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(DTC_OVERLAY_FILE ${CMAKE_CURRENT_SOURCE_DIR}/boards/rpi_pico2_rp2350a_m33.overlay)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(generic_i2c)

target_sources(app PRIVATE 
src/main.c 
src/gpio_i2c.c 
src/gpio_i2c.h
src/spio.c
src/spio.h
src/tones.c
src/tones.h
src/main_fsm.c
src/main_fsm.h
        src/slic.c
        src/slic.h
)
