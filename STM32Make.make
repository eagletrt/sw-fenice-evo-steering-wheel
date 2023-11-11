##########################################################################################################################
# File automatically-generated by STM32forVSCode
##########################################################################################################################

# ------------------------------------------------
# Generic Makefile (based on gcc)
#
# ChangeLog :
#	2017-02-10 - Several enhancements + project update mode
#   2015-07-22 - first version
# ------------------------------------------------

######################################
# target
######################################
TARGET = fenice-evo-steering-wheel


######################################
# building variables
######################################
# debug build?
DEBUG = 1
# optimization
OPT = -Os


#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

######################################
# source
######################################
# C sources
C_SOURCES =  \
Core/Lib/can/lib/bms/bms_network.c \
Core/Lib/can/lib/bms/bms_watchdog.c \
Core/Lib/can/lib/inverters/inverters_network.c \
Core/Lib/can/lib/inverters/inverters_watchdog.c \
Core/Lib/can/lib/primary/primary_network.c \
Core/Lib/can/lib/primary/primary_watchdog.c \
Core/Lib/can/lib/secondary/secondary_network.c \
Core/Lib/can/lib/secondary/secondary_watchdog.c \
Core/Lib/lvgl-stm32/lvgl/src/core/lv_disp.c \
Core/Lib/lvgl-stm32/lvgl/src/core/lv_event.c \
Core/Lib/lvgl-stm32/lvgl/src/core/lv_group.c \
Core/Lib/lvgl-stm32/lvgl/src/core/lv_indev.c \
Core/Lib/lvgl-stm32/lvgl/src/core/lv_indev_scroll.c \
Core/Lib/lvgl-stm32/lvgl/src/core/lv_obj.c \
Core/Lib/lvgl-stm32/lvgl/src/core/lv_obj_class.c \
Core/Lib/lvgl-stm32/lvgl/src/core/lv_obj_draw.c \
Core/Lib/lvgl-stm32/lvgl/src/core/lv_obj_pos.c \
Core/Lib/lvgl-stm32/lvgl/src/core/lv_obj_scroll.c \
Core/Lib/lvgl-stm32/lvgl/src/core/lv_obj_style.c \
Core/Lib/lvgl-stm32/lvgl/src/core/lv_obj_style_gen.c \
Core/Lib/lvgl-stm32/lvgl/src/core/lv_obj_tree.c \
Core/Lib/lvgl-stm32/lvgl/src/core/lv_refr.c \
Core/Lib/lvgl-stm32/lvgl/src/core/lv_theme.c \
Core/Lib/lvgl-stm32/lvgl/src/draw/lv_draw_arc.c \
Core/Lib/lvgl-stm32/lvgl/src/draw/lv_draw_blend.c \
Core/Lib/lvgl-stm32/lvgl/src/draw/lv_draw_img.c \
Core/Lib/lvgl-stm32/lvgl/src/draw/lv_draw_label.c \
Core/Lib/lvgl-stm32/lvgl/src/draw/lv_draw_line.c \
Core/Lib/lvgl-stm32/lvgl/src/draw/lv_draw_mask.c \
Core/Lib/lvgl-stm32/lvgl/src/draw/lv_draw_rect.c \
Core/Lib/lvgl-stm32/lvgl/src/draw/lv_draw_triangle.c \
Core/Lib/lvgl-stm32/lvgl/src/draw/lv_img_buf.c \
Core/Lib/lvgl-stm32/lvgl/src/draw/lv_img_cache.c \
Core/Lib/lvgl-stm32/lvgl/src/draw/lv_img_decoder.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/layouts/flex/lv_flex.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/layouts/grid/lv_grid.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/libs/bmp/lv_bmp.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/libs/freetype/lv_freetype.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/libs/fsdrv/lv_fs_fatfs.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/libs/fsdrv/lv_fs_posix.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/libs/fsdrv/lv_fs_stdio.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/libs/fsdrv/lv_fs_win32.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/libs/gif/gifdec.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/libs/gif/lv_gif.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/libs/png/lodepng.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/libs/png/lv_png.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/libs/qrcode/lv_qrcode.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/libs/qrcode/qrcodegen.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/libs/rlottie/lv_rlottie.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/libs/sjpg/lv_sjpg.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/libs/sjpg/tjpgd.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/lv_extra.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/others/snapshot/lv_snapshot.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/themes/basic/lv_theme_basic.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/themes/default/lv_theme_default.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/themes/mono/lv_theme_mono.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/widgets/animimg/lv_animimg.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/widgets/calendar/lv_calendar.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/widgets/calendar/lv_calendar_header_arrow.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/widgets/calendar/lv_calendar_header_dropdown.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/widgets/chart/lv_chart.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/widgets/colorwheel/lv_colorwheel.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/widgets/imgbtn/lv_imgbtn.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/widgets/keyboard/lv_keyboard.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/widgets/led/lv_led.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/widgets/list/lv_list.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/widgets/meter/lv_meter.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/widgets/msgbox/lv_msgbox.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/widgets/span/lv_span.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/widgets/spinbox/lv_spinbox.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/widgets/spinner/lv_spinner.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/widgets/tabview/lv_tabview.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/widgets/tileview/lv_tileview.c \
Core/Lib/lvgl-stm32/lvgl/src/extra/widgets/win/lv_win.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_dejavu_16_persian_hebrew.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_fmt_txt.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_loader.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_montserrat_10.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_montserrat_12.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_montserrat_12_subpx.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_montserrat_14.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_montserrat_16.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_montserrat_18.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_montserrat_20.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_montserrat_22.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_montserrat_24.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_montserrat_26.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_montserrat_28.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_montserrat_28_compressed.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_montserrat_30.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_montserrat_32.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_montserrat_34.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_montserrat_36.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_montserrat_38.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_montserrat_40.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_montserrat_42.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_montserrat_44.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_montserrat_46.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_montserrat_48.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_montserrat_8.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_simsun_16_cjk.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_unscii_16.c \
Core/Lib/lvgl-stm32/lvgl/src/font/lv_font_unscii_8.c \
Core/Lib/lvgl-stm32/lvgl/src/gpu/lv_gpu_nxp_pxp.c \
Core/Lib/lvgl-stm32/lvgl/src/gpu/lv_gpu_nxp_pxp_osa.c \
Core/Lib/lvgl-stm32/lvgl/src/gpu/lv_gpu_nxp_vglite.c \
Core/Lib/lvgl-stm32/lvgl/src/gpu/lv_gpu_sdl.c \
Core/Lib/lvgl-stm32/lvgl/src/gpu/lv_gpu_stm32_dma2d.c \
Core/Lib/lvgl-stm32/lvgl/src/gpu/sdl/lv_gpu_sdl_draw_arc.c \
Core/Lib/lvgl-stm32/lvgl/src/gpu/sdl/lv_gpu_sdl_draw_blend.c \
Core/Lib/lvgl-stm32/lvgl/src/gpu/sdl/lv_gpu_sdl_draw_img.c \
Core/Lib/lvgl-stm32/lvgl/src/gpu/sdl/lv_gpu_sdl_draw_label.c \
Core/Lib/lvgl-stm32/lvgl/src/gpu/sdl/lv_gpu_sdl_draw_line.c \
Core/Lib/lvgl-stm32/lvgl/src/gpu/sdl/lv_gpu_sdl_draw_rect.c \
Core/Lib/lvgl-stm32/lvgl/src/gpu/sdl/lv_gpu_sdl_lru.c \
Core/Lib/lvgl-stm32/lvgl/src/gpu/sdl/lv_gpu_sdl_mask.c \
Core/Lib/lvgl-stm32/lvgl/src/gpu/sdl/lv_gpu_sdl_stack_blur.c \
Core/Lib/lvgl-stm32/lvgl/src/gpu/sdl/lv_gpu_sdl_texture_cache.c \
Core/Lib/lvgl-stm32/lvgl/src/gpu/sdl/lv_gpu_sdl_utils.c \
Core/Lib/lvgl-stm32/lvgl/src/hal/lv_hal_disp.c \
Core/Lib/lvgl-stm32/lvgl/src/hal/lv_hal_indev.c \
Core/Lib/lvgl-stm32/lvgl/src/hal/lv_hal_tick.c \
Core/Lib/lvgl-stm32/lvgl/src/misc/lv_anim.c \
Core/Lib/lvgl-stm32/lvgl/src/misc/lv_anim_timeline.c \
Core/Lib/lvgl-stm32/lvgl/src/misc/lv_area.c \
Core/Lib/lvgl-stm32/lvgl/src/misc/lv_async.c \
Core/Lib/lvgl-stm32/lvgl/src/misc/lv_bidi.c \
Core/Lib/lvgl-stm32/lvgl/src/misc/lv_color.c \
Core/Lib/lvgl-stm32/lvgl/src/misc/lv_fs.c \
Core/Lib/lvgl-stm32/lvgl/src/misc/lv_gc.c \
Core/Lib/lvgl-stm32/lvgl/src/misc/lv_ll.c \
Core/Lib/lvgl-stm32/lvgl/src/misc/lv_log.c \
Core/Lib/lvgl-stm32/lvgl/src/misc/lv_math.c \
Core/Lib/lvgl-stm32/lvgl/src/misc/lv_mem.c \
Core/Lib/lvgl-stm32/lvgl/src/misc/lv_printf.c \
Core/Lib/lvgl-stm32/lvgl/src/misc/lv_style.c \
Core/Lib/lvgl-stm32/lvgl/src/misc/lv_style_gen.c \
Core/Lib/lvgl-stm32/lvgl/src/misc/lv_templ.c \
Core/Lib/lvgl-stm32/lvgl/src/misc/lv_timer.c \
Core/Lib/lvgl-stm32/lvgl/src/misc/lv_tlsf.c \
Core/Lib/lvgl-stm32/lvgl/src/misc/lv_txt.c \
Core/Lib/lvgl-stm32/lvgl/src/misc/lv_txt_ap.c \
Core/Lib/lvgl-stm32/lvgl/src/misc/lv_utils.c \
Core/Lib/lvgl-stm32/lvgl/src/widgets/lv_arc.c \
Core/Lib/lvgl-stm32/lvgl/src/widgets/lv_bar.c \
Core/Lib/lvgl-stm32/lvgl/src/widgets/lv_btn.c \
Core/Lib/lvgl-stm32/lvgl/src/widgets/lv_btnmatrix.c \
Core/Lib/lvgl-stm32/lvgl/src/widgets/lv_canvas.c \
Core/Lib/lvgl-stm32/lvgl/src/widgets/lv_checkbox.c \
Core/Lib/lvgl-stm32/lvgl/src/widgets/lv_dropdown.c \
Core/Lib/lvgl-stm32/lvgl/src/widgets/lv_img.c \
Core/Lib/lvgl-stm32/lvgl/src/widgets/lv_label.c \
Core/Lib/lvgl-stm32/lvgl/src/widgets/lv_line.c \
Core/Lib/lvgl-stm32/lvgl/src/widgets/lv_objx_templ.c \
Core/Lib/lvgl-stm32/lvgl/src/widgets/lv_roller.c \
Core/Lib/lvgl-stm32/lvgl/src/widgets/lv_slider.c \
Core/Lib/lvgl-stm32/lvgl/src/widgets/lv_switch.c \
Core/Lib/lvgl-stm32/lvgl/src/widgets/lv_table.c \
Core/Lib/lvgl-stm32/lvgl/src/widgets/lv_textarea.c \
Core/Src/dac.c \
Core/Src/dma2d.c \
Core/Src/fdcan.c \
Core/Src/fmc.c \
Core/Src/gpio.c \
Core/Src/i2c.c \
Core/Src/inputs/inputs.c \
Core/Src/inputs/mcp23017.c \
Core/Src/led_control.c \
Core/Src/ltdc.c \
Core/Src/main.c \
Core/Src/memdriver.c \
Core/Src/octospi.c \
Core/Src/screen_driver.c \
Core/Src/stm32h7xx_hal_msp.c \
Core/Src/stm32h7xx_it.c \
Core/Src/syscalls.c \
Core/Src/sysmem.c \
Core/Src/system_stm32h7xx.c \
Core/Src/test/i2c_test.c \
Core/Src/test/sdram_test.c \
Core/Src/tim.c \
Core/Src/usart.c \
Core/Src/watchdog.c \
Core/UI/steering/can_messages.c \
Core/UI/steering/cansniffer.c \
Core/UI/steering/controls.c \
Core/UI/steering/engineer_mode/tab_balancing_status.c \
Core/UI/steering/engineer_mode/tab_cooling.c \
Core/UI/steering/engineer_mode/tab_engineer_mode.c \
Core/UI/steering/engineer_mode/tab_fatal_error.c \
Core/UI/steering/engineer_mode/tab_primary_cansniffer.c \
Core/UI/steering/engineer_mode/tab_secondary_cansniffer.c \
Core/UI/steering/engineer_mode/tab_shutdown_circuit.c \
Core/UI/steering/engineer_mode/tab_terminal.c \
Core/UI/steering/fonts/lv_font_inter_bold_14.c \
Core/UI/steering/fonts/lv_font_inter_bold_18.c \
Core/UI/steering/fonts/lv_font_inter_bold_20.c \
Core/UI/steering/fonts/lv_font_inter_bold_22.c \
Core/UI/steering/fonts/lv_font_inter_bold_30.c \
Core/UI/steering/fonts/lv_font_inter_bold_38.c \
Core/UI/steering/fonts/lv_font_inter_bold_70.c \
Core/UI/steering/graphics_manager.c \
Core/UI/steering/maxheap.c \
Core/UI/steering/racing_tabs/tab_calibration.c \
Core/UI/steering/racing_tabs/tab_debug.c \
Core/UI/steering/racing_tabs/tab_notification.c \
Core/UI/steering/racing_tabs/tab_racing.c \
Core/UI/steering/racing_tabs/tab_sensors.c \
Core/UI/steering/racing_tabs/tab_track_test.c \
Core/UI/steering/steering.c \
Core/UI/steering/tab_manager.c \
Core/UI/steering/utils.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma2d.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_exti.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_fdcan.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hsem.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ospi.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sdram.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart_ex.c \
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_fmc.c


CPP_SOURCES = \


# ASM sources
ASM_SOURCES =  \
startup_stm32h723xx.s



#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
POSTFIX = "
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
GCC_PATH="/usr/bin
ifdef GCC_PATH
CXX = $(GCC_PATH)/$(PREFIX)g++$(POSTFIX)
CC = $(GCC_PATH)/$(PREFIX)gcc$(POSTFIX)
AS = $(GCC_PATH)/$(PREFIX)gcc$(POSTFIX) -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy$(POSTFIX)
SZ = $(GCC_PATH)/$(PREFIX)size$(POSTFIX)
else
CXX = $(PREFIX)g++
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m7

# fpu
FPU = -mfpu=fpv5-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS = 

# C defines
C_DEFS =  \
-DLV_LVGL_H_INCLUDE_SIMPLE \
-DSTM32H723xx \
-DUSE_HAL_DRIVER


# CXX defines
CXX_DEFS =  \
-DSTM32H723xx \
-DUSE_HAL_DRIVER


# AS includes
AS_INCLUDES = \

# C includes
C_INCLUDES =  \
-ICore/Inc \
-ICore/Inc/inputs \
-ICore/Inc/test \
-ICore/Lib/can/lib \
-ICore/Lib/lvgl-stm32/lvgl \
-ICore/UI/steering \
-IDrivers/CMSIS/Device/ST/STM32H7xx/Include \
-IDrivers/CMSIS/Include \
-IDrivers/STM32H7xx_HAL_Driver/Inc \
-IDrivers/STM32H7xx_HAL_Driver/Inc/Legacy



# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CXXFLAGS = $(MCU) $(CXX_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections -feliminate-unused-debug-types

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf -ggdb
CXXFLAGS += -g -gdwarf -ggdb
endif

# Add additional flags
CFLAGS += 
ASFLAGS += 
CXXFLAGS += 

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"
CXXFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"

#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = STM32H723ZGTx_FLASH_shifted.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = \


# Additional LD Flags from config file
ADDITIONALLDFLAGS = -specs=nano.specs -u_printf_float 

LDFLAGS = $(MCU) $(ADDITIONALLDFLAGS) -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of cpp program objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(CPP_SOURCES:.cpp=.o)))
vpath %.cpp $(sort $(dir $(CPP_SOURCES)))

# list of C objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))

# list of ASM program objects
# list of ASM program objects
UPPER_CASE_ASM_SOURCES = $(filter %.S,$(ASM_SOURCES))
LOWER_CASE_ASM_SOURCES = $(filter %.s,$(ASM_SOURCES))

OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(UPPER_CASE_ASM_SOURCES:.S=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(LOWER_CASE_ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.cpp STM32Make.make | $(BUILD_DIR) 
	$(CXX) -c $(CXXFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.cpp=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.cxx STM32Make.make | $(BUILD_DIR) 
	$(CXX) -c $(CXXFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.cxx=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.c STM32Make.make | $(BUILD_DIR) 
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s STM32Make.make | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.S STM32Make.make | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) STM32Make.make
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@

$(BUILD_DIR):
	mkdir $@

#######################################
# flash
#######################################
flash: $(BUILD_DIR)/$(TARGET).elf
	"/usr/bin/openocd" -f ./openocd.cfg -c "program $(BUILD_DIR)/$(TARGET).elf verify reset exit"

#######################################
# erase
#######################################
erase: $(BUILD_DIR)/$(TARGET).elf
	"/usr/bin/openocd" -f ./openocd.cfg -c "init; reset halt; stm32h7x mass_erase 0; exit"

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)

#######################################
# custom makefile rules
#######################################


	
#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***