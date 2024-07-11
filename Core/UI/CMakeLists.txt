CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
SET(CMAKE_CXX_STANDARD 17)

PROJECT(SteeringSimulator)

ADD_COMPILE_DEFINITIONS(SIMULATOR_CAN)

file(GLOB_RECURSE MIN_HEAP_SOURCES ../Lib/micro-libs/min-heap/src/min-heap.c)
file(GLOB_RECURSE LVGL_SOURCES ../Lib/lvgl-linux/lvgl/**/*.c)
file(GLOB_RECURSE LVDRIVERS_SOURCES ../Lib/lvgl-linux/lv-drivers/**/*.c)
file(GLOB_RECURSE CANLIB_SOURCES ../Lib/can/lib/**/*.c)

add_executable(SteeringSimulator 
    steering/steering.c
    steering/tab_manager.c
    steering/engineer_mode/tab_fatal_error.c
    steering/engineer_mode/tab_terminal.c
    steering/engineer_mode/tab_debug.c
    steering/racing_tabs/tab_notification.c
    steering/racing_tabs/tab_racing.c
    steering/racing_tabs/tab_sensors.c
    steering/racing_tabs/tab_hv.c
    steering/racing_tabs/tab_lv.c
    steering/racing_tabs/tab_track_test.c
    steering/maxheap.c
    steering/engineer_mode/tab_primary_cansniffer.c
    steering/engineer_mode/tab_secondary_cansniffer.c
    steering/engineer_mode/tab_inverters_cansniffer.c
    steering/utils.c
    steering/controls.c
    steering/can_messages.c
    steering/cansniffer.c
    steering/fonts/lv_font_inter_bold_14.c
    steering/fonts/lv_font_inter_bold_18.c
    steering/fonts/lv_font_inter_bold_20.c
    steering/fonts/lv_font_inter_bold_22.c
    steering/fonts/lv_font_inter_bold_30.c
    steering/fonts/lv_font_inter_bold_38.c
    steering/fonts/lv_font_inter_bold_70.c
    simulator/can.c
    simulator/main.c
    simulator/mouse_cursor_icon.c
    simulator/queue.c
    steering/watchdog.c
    ${LVGL_SOURCES}
    ${LVDRIVERS_SOURCES}
    ${CANLIB_SOURCES}
    ${MIN_HEAP_SOURCES}
    steering/graphics_manager.c
)

add_definitions(-DLV_LVGL_H_INCLUDE_SIMPLE)

TARGET_INCLUDE_DIRECTORIES(SteeringSimulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Lib/micro-libs/min-heap/inc)
TARGET_INCLUDE_DIRECTORIES(SteeringSimulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Lib/can/lib)
TARGET_INCLUDE_DIRECTORIES(SteeringSimulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Lib/lvgl-linux/lvgl)
TARGET_INCLUDE_DIRECTORIES(SteeringSimulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Lib/lvgl-linux)
TARGET_INCLUDE_DIRECTORIES(SteeringSimulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/steering)

TARGET_LINK_LIBRARIES(SteeringSimulator pthread SDL2 m)
