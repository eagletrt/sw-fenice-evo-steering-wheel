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
OPT = -Og


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
Core/Src/dac.c \
Core/Src/display_buf.c \
Core/Src/dma2d.c \
Core/Src/fdcan.c \
Core/Src/fmc.c \
Core/Src/fonts/lv_font_inter_bold_14.c \
Core/Src/fonts/lv_font_inter_bold_18.c \
Core/Src/fonts/lv_font_inter_bold_22.c \
Core/Src/fonts/lv_font_inter_bold_30.c \
Core/Src/fonts/lv_font_inter_bold_38.c \
Core/Src/fonts/lv_font_inter_bold_60.c \
Core/Src/fonts/lv_font_inter_bold_70.c \
Core/Src/gpio.c \
Core/Src/i2c.c \
Core/Src/led_control.c \
Core/Src/ltdc.c \
Core/Src/lvgl_port/lvgl_port.c \
Core/Src/lvgl_port/screen_driver.c \
Core/Src/main.c \
Core/Src/memdriver.c \
Core/Src/octospi.c \
Core/Src/steering/steering.c \
Core/Src/steering/tab_calibration.c \
Core/Src/steering/tab_debug.c \
Core/Src/steering/tab_manager.c \
Core/Src/steering/tab_racing.c \
Core/Src/steering/utils.c \
Core/Src/stm32h7xx_hal_msp.c \
Core/Src/stm32h7xx_it.c \
Core/Src/syscalls.c \
Core/Src/sysmem.c \
Core/Src/system_stm32h7xx.c \
Core/Src/test/i2c_test.c \
Core/Src/test/sdram_test.c \
Core/Src/test/uart_test.c \
Core/Src/tim.c \
Core/Src/usart.c \
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
Drivers/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_fmc.c \
lvgl/src/core/lv_disp.c \
lvgl/src/core/lv_event.c \
lvgl/src/core/lv_group.c \
lvgl/src/core/lv_indev.c \
lvgl/src/core/lv_indev_scroll.c \
lvgl/src/core/lv_obj.c \
lvgl/src/core/lv_obj_class.c \
lvgl/src/core/lv_obj_draw.c \
lvgl/src/core/lv_obj_pos.c \
lvgl/src/core/lv_obj_scroll.c \
lvgl/src/core/lv_obj_style.c \
lvgl/src/core/lv_obj_style_gen.c \
lvgl/src/core/lv_obj_tree.c \
lvgl/src/core/lv_refr.c \
lvgl/src/core/lv_theme.c \
lvgl/src/draw/lv_draw.c \
lvgl/src/draw/lv_draw_arc.c \
lvgl/src/draw/lv_draw_img.c \
lvgl/src/draw/lv_draw_label.c \
lvgl/src/draw/lv_draw_line.c \
lvgl/src/draw/lv_draw_mask.c \
lvgl/src/draw/lv_draw_rect.c \
lvgl/src/draw/lv_draw_triangle.c \
lvgl/src/draw/lv_img_buf.c \
lvgl/src/draw/lv_img_cache.c \
lvgl/src/draw/lv_img_decoder.c \
lvgl/src/draw/nxp_pxp/lv_gpu_nxp_pxp.c \
lvgl/src/draw/nxp_pxp/lv_gpu_nxp_pxp_osa.c \
lvgl/src/draw/nxp_vglite/lv_gpu_nxp_vglite.c \
lvgl/src/draw/sdl/lv_draw_sdl.c \
lvgl/src/draw/sdl/lv_draw_sdl_arc.c \
lvgl/src/draw/sdl/lv_draw_sdl_bg.c \
lvgl/src/draw/sdl/lv_draw_sdl_composite.c \
lvgl/src/draw/sdl/lv_draw_sdl_img.c \
lvgl/src/draw/sdl/lv_draw_sdl_label.c \
lvgl/src/draw/sdl/lv_draw_sdl_line.c \
lvgl/src/draw/sdl/lv_draw_sdl_mask.c \
lvgl/src/draw/sdl/lv_draw_sdl_polygon.c \
lvgl/src/draw/sdl/lv_draw_sdl_rect.c \
lvgl/src/draw/sdl/lv_draw_sdl_stack_blur.c \
lvgl/src/draw/sdl/lv_draw_sdl_texture_cache.c \
lvgl/src/draw/sdl/lv_draw_sdl_utils.c \
lvgl/src/draw/stm32_dma2d/lv_gpu_stm32_dma2d.c \
lvgl/src/draw/sw/lv_draw_sw.c \
lvgl/src/draw/sw/lv_draw_sw_arc.c \
lvgl/src/draw/sw/lv_draw_sw_blend.c \
lvgl/src/draw/sw/lv_draw_sw_dither.c \
lvgl/src/draw/sw/lv_draw_sw_gradient.c \
lvgl/src/draw/sw/lv_draw_sw_img.c \
lvgl/src/draw/sw/lv_draw_sw_letter.c \
lvgl/src/draw/sw/lv_draw_sw_line.c \
lvgl/src/draw/sw/lv_draw_sw_polygon.c \
lvgl/src/draw/sw/lv_draw_sw_rect.c \
lvgl/src/extra/layouts/flex/lv_flex.c \
lvgl/src/extra/layouts/grid/lv_grid.c \
lvgl/src/extra/libs/bmp/lv_bmp.c \
lvgl/src/extra/libs/ffmpeg/lv_ffmpeg.c \
lvgl/src/extra/libs/freetype/lv_freetype.c \
lvgl/src/extra/libs/fsdrv/lv_fs_fatfs.c \
lvgl/src/extra/libs/fsdrv/lv_fs_posix.c \
lvgl/src/extra/libs/fsdrv/lv_fs_stdio.c \
lvgl/src/extra/libs/fsdrv/lv_fs_win32.c \
lvgl/src/extra/libs/gif/gifdec.c \
lvgl/src/extra/libs/gif/lv_gif.c \
lvgl/src/extra/libs/png/lodepng.c \
lvgl/src/extra/libs/png/lv_png.c \
lvgl/src/extra/libs/qrcode/lv_qrcode.c \
lvgl/src/extra/libs/qrcode/qrcodegen.c \
lvgl/src/extra/libs/rlottie/lv_rlottie.c \
lvgl/src/extra/libs/sjpg/lv_sjpg.c \
lvgl/src/extra/libs/sjpg/tjpgd.c \
lvgl/src/extra/lv_extra.c \
lvgl/src/extra/others/gridnav/lv_gridnav.c \
lvgl/src/extra/others/monkey/lv_monkey.c \
lvgl/src/extra/others/snapshot/lv_snapshot.c \
lvgl/src/extra/themes/basic/lv_theme_basic.c \
lvgl/src/extra/themes/default/lv_theme_default.c \
lvgl/src/extra/themes/mono/lv_theme_mono.c \
lvgl/src/extra/widgets/animimg/lv_animimg.c \
lvgl/src/extra/widgets/calendar/lv_calendar.c \
lvgl/src/extra/widgets/calendar/lv_calendar_header_arrow.c \
lvgl/src/extra/widgets/calendar/lv_calendar_header_dropdown.c \
lvgl/src/extra/widgets/chart/lv_chart.c \
lvgl/src/extra/widgets/colorwheel/lv_colorwheel.c \
lvgl/src/extra/widgets/imgbtn/lv_imgbtn.c \
lvgl/src/extra/widgets/keyboard/lv_keyboard.c \
lvgl/src/extra/widgets/led/lv_led.c \
lvgl/src/extra/widgets/list/lv_list.c \
lvgl/src/extra/widgets/menu/lv_menu.c \
lvgl/src/extra/widgets/meter/lv_meter.c \
lvgl/src/extra/widgets/msgbox/lv_msgbox.c \
lvgl/src/extra/widgets/span/lv_span.c \
lvgl/src/extra/widgets/spinbox/lv_spinbox.c \
lvgl/src/extra/widgets/spinner/lv_spinner.c \
lvgl/src/extra/widgets/tabview/lv_tabview.c \
lvgl/src/extra/widgets/tileview/lv_tileview.c \
lvgl/src/extra/widgets/win/lv_win.c \
lvgl/src/font/lv_font.c \
lvgl/src/font/lv_font_dejavu_16_persian_hebrew.c \
lvgl/src/font/lv_font_fmt_txt.c \
lvgl/src/font/lv_font_loader.c \
lvgl/src/font/lv_font_montserrat_10.c \
lvgl/src/font/lv_font_montserrat_12.c \
lvgl/src/font/lv_font_montserrat_12_subpx.c \
lvgl/src/font/lv_font_montserrat_14.c \
lvgl/src/font/lv_font_montserrat_16.c \
lvgl/src/font/lv_font_montserrat_18.c \
lvgl/src/font/lv_font_montserrat_20.c \
lvgl/src/font/lv_font_montserrat_22.c \
lvgl/src/font/lv_font_montserrat_24.c \
lvgl/src/font/lv_font_montserrat_26.c \
lvgl/src/font/lv_font_montserrat_28.c \
lvgl/src/font/lv_font_montserrat_28_compressed.c \
lvgl/src/font/lv_font_montserrat_30.c \
lvgl/src/font/lv_font_montserrat_32.c \
lvgl/src/font/lv_font_montserrat_34.c \
lvgl/src/font/lv_font_montserrat_36.c \
lvgl/src/font/lv_font_montserrat_38.c \
lvgl/src/font/lv_font_montserrat_40.c \
lvgl/src/font/lv_font_montserrat_42.c \
lvgl/src/font/lv_font_montserrat_44.c \
lvgl/src/font/lv_font_montserrat_46.c \
lvgl/src/font/lv_font_montserrat_48.c \
lvgl/src/font/lv_font_montserrat_8.c \
lvgl/src/font/lv_font_simsun_16_cjk.c \
lvgl/src/font/lv_font_unscii_16.c \
lvgl/src/font/lv_font_unscii_8.c \
lvgl/src/hal/lv_hal_disp.c \
lvgl/src/hal/lv_hal_indev.c \
lvgl/src/hal/lv_hal_tick.c \
lvgl/src/misc/lv_anim.c \
lvgl/src/misc/lv_anim_timeline.c \
lvgl/src/misc/lv_area.c \
lvgl/src/misc/lv_async.c \
lvgl/src/misc/lv_bidi.c \
lvgl/src/misc/lv_color.c \
lvgl/src/misc/lv_fs.c \
lvgl/src/misc/lv_gc.c \
lvgl/src/misc/lv_ll.c \
lvgl/src/misc/lv_log.c \
lvgl/src/misc/lv_lru.c \
lvgl/src/misc/lv_math.c \
lvgl/src/misc/lv_mem.c \
lvgl/src/misc/lv_printf.c \
lvgl/src/misc/lv_style.c \
lvgl/src/misc/lv_style_gen.c \
lvgl/src/misc/lv_templ.c \
lvgl/src/misc/lv_timer.c \
lvgl/src/misc/lv_tlsf.c \
lvgl/src/misc/lv_txt.c \
lvgl/src/misc/lv_txt_ap.c \
lvgl/src/misc/lv_utils.c \
lvgl/src/widgets/lv_arc.c \
lvgl/src/widgets/lv_bar.c \
lvgl/src/widgets/lv_btn.c \
lvgl/src/widgets/lv_btnmatrix.c \
lvgl/src/widgets/lv_canvas.c \
lvgl/src/widgets/lv_checkbox.c \
lvgl/src/widgets/lv_dropdown.c \
lvgl/src/widgets/lv_img.c \
lvgl/src/widgets/lv_label.c \
lvgl/src/widgets/lv_line.c \
lvgl/src/widgets/lv_objx_templ.c \
lvgl/src/widgets/lv_roller.c \
lvgl/src/widgets/lv_slider.c \
lvgl/src/widgets/lv_switch.c \
lvgl/src/widgets/lv_table.c \
lvgl/src/widgets/lv_textarea.c


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
-ICore/Inc/lvgl_port \
-ICore/Inc/steering \
-ICore/Inc/test \
-IDrivers/CMSIS/Device/ST/STM32H7xx/Include \
-IDrivers/CMSIS/Include \
-IDrivers/STM32H7xx_HAL_Driver/Inc \
-IDrivers/STM32H7xx_HAL_Driver/Inc/Legacy \
-Ilvgl



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
LDSCRIPT = STM32H723ZGTx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys 
LIBDIR = \


# Additional LD Flags from config file
ADDITIONALLDFLAGS = -specs=nano.specs 

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