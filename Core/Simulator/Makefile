# Compiler and flags
FONTDIR = ../UI/Lib/McuFont/fonts
MFDIR = ../UI/Lib/McuFont/decoder
include $(MFDIR)/mcufont.mk

CC = gcc
CFLAGS = -Wall
INCLUDES = -I../UI/Inc -I../Lib/can/lib/ -I../Lib/invlib/ -I$(FONTDIR) -I$(MFDIR) -lSDL2 -lm
DEBUG_FLAGS = -g -ggdb

BUILD_DIR = build

# Source files
SRC = vc.c queue.c can.c \
    ../UI/Src/can_messages.c \
    ../UI/Src/controls.c \
    ../UI/Src/graphics_manager.c \
    ../UI/Src/maxheap.c \
    ../UI/Src/steering.c \
    ../UI/Src/watchdog.c \
    ../Lib/can/lib/bms/bms_network.c \
    ../Lib/can/lib/bms/bms_utils.c \
    ../Lib/can/lib/bms/bms_watchdog.c \
    ../Lib/can/lib/canlib_device.c \
    ../Lib/can/lib/inverters/inverters_network.c \
    ../Lib/can/lib/inverters/inverters_utils.c \
    ../Lib/can/lib/inverters/inverters_watchdog.c \
    ../Lib/can/lib/ivts/ivts_network.c \
    ../Lib/can/lib/ivts/ivts_utils.c \
    ../Lib/can/lib/ivts/ivts_watchdog.c \
    ../Lib/can/lib/primary/primary_network.c \
    ../Lib/can/lib/primary/primary_utils.c \
    ../Lib/can/lib/primary/primary_watchdog.c \
    ../Lib/can/lib/secondary/secondary_network.c \
    ../Lib/can/lib/secondary/secondary_utils.c \
    ../Lib/can/lib/secondary/secondary_watchdog.c \
    ../Lib/can/lib/simulator/simulator_network.c \
    ../Lib/can/lib/simulator/simulator_utils.c \
    ../Lib/can/lib/simulator/simulator_watchdog.c \
    ../Lib/invlib/inverter_conversions.c

# Output binary
TARGET = simulator.out

OBJ = $(patsubst %.c,$(BUILD_DIR)/%.o,$(notdir $(SRC)))

vpath %.c ../UI/Src ../Lib/can/lib/bms ../Lib/can/lib ../Lib/can/lib/inverters ../Lib/can/lib/ivts ../Lib/can/lib/primary ../Lib/can/lib/secondary ../Lib/can/lib/simulator ../Lib/invlib

all: $(TARGET)
	@echo "Target up to date"

# Build with debug options
debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) $(MFSRC) $(INCLUDES) -o $(TARGET)

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR) 
	$(CC) $(CFLAGS) -c $< -o $@ $(INCLUDES)

$(BUILD_DIR):
	mkdir $@

run: $(TARGET)
	./$(TARGET)

# Clean up build files
clean:
	rm -f $(TARGET)
	rm -rf $(BUILD_DIR)
