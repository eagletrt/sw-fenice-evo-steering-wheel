CMAKE_MINIMUM_REQUIRED(VERSION 3.0)
SET(CMAKE_CXX_STANDARD 17)

PROJECT(SteeringSimulator)

file(GLOB_RECURSE LVGL_SOURCES ../Lib/lvgl-linux/lvgl/**/*.c)
file(GLOB_RECURSE LVDRIVERS_SOURCES ../Lib/lvgl-linux/lv-drivers/**/*.c)
file(GLOB_RECURSE CANLIB_SOURCES ../Lib/can/lib/**/*.c)

add_executable(SteeringSimulator 
    steering/steering.c
    steering/tab_manager.c
    steering/engineer_mode/tab_engineer_mode.c
    steering/engineer_mode/tab_cooling.c
    steering/racing_tabs/tab_calibration.c
    steering/racing_tabs/tab_debug.c
    steering/racing_tabs/tab_notification.c
    steering/racing_tabs/tab_racing.c
    steering/racing_tabs/tab_sensors.c
    steering/racing_tabs/tab_track_test.c
    steering/cansniffer.c
    steering/engineer_mode/tab_primary_cansniffer.c
    steering/utils.c
    steering/controls.c
    steering/can_messages.c
    steering/fonts/lv_font_inter_bold_14.c
    steering/fonts/lv_font_inter_bold_18.c
    steering/fonts/lv_font_inter_bold_20.c
    steering/fonts/lv_font_inter_bold_22.c
    steering/fonts/lv_font_inter_bold_30.c
    steering/fonts/lv_font_inter_bold_38.c
    simulator/can.c
    simulator/main.c
    simulator/mouse_cursor_icon.c
    simulator/queue.c
    ${LVGL_SOURCES}
    ${LVDRIVERS_SOURCES}
    ${CANLIB_SOURCES}
)

add_definitions(-DLV_LVGL_H_INCLUDE_SIMPLE)

TARGET_INCLUDE_DIRECTORIES(SteeringSimulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Lib/can/lib)
TARGET_INCLUDE_DIRECTORIES(SteeringSimulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Lib/lvgl-linux/lvgl)
TARGET_INCLUDE_DIRECTORIES(SteeringSimulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Lib/lvgl-linux)
TARGET_INCLUDE_DIRECTORIES(SteeringSimulator PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/steering)

TARGET_LINK_LIBRARIES(SteeringSimulator pthread SDL2)