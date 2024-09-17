# Compiler and flags
CC = gcc
CFLAGS = -Wall
INCLUDES = -I../UI/Inc -I../Lib/can/lib/ -I../Lib/invlib/ -lSDL2 -lm
DEBUG_FLAGS = -g -ggdb

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

# TODO: if there is yet a.out if you type make it gives all ok even if you changed the source files
# Output binary
TARGET = a.out

# Build with debug option
all: $(TARGET)

debug: CFLAGS += $(DEBUG_FLAGS)
debug: $(TARGET)

$(TARGET):
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET) $(INCLUDES)

run: $(TARGET)
	./$(TARGET)

# Clean up build files
clean:
	rm $(TARGET)
