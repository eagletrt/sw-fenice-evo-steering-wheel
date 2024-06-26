/**
 * @file mcp23017.c
 * @author Tommaso Canova [tommaso.canova@studenti.unitn.it]]
 * @brief
 * @version 0.1
 * @date 2022-05-04
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef MCP23017_H
#define MCP23017_H

#include "main.h"

// Registers
#define REGISTER_IODIRA   0x00
#define REGISTER_IODIRB   0x01
#define REGISTER_IPOLA    0x02
#define REGISTER_IPOLB    0x03
#define REGISTER_GPINTENA 0x04
#define REGISTER_GPINTENB 0x05
#define REGISTER_DEFVALA  0x06
#define REGISTER_DEFVALB  0x07
#define REGISTER_INTCONA  0x08
#define REGISTER_INTCONB  0x09
//	IOCON			0x0A
//	IOCON			0x0B
#define REGISTER_GPPUA   0x0C
#define REGISTER_GPPUB   0x0D
#define REGISTER_INTFA   0x0E
#define REGISTER_INTFB   0x0F
#define REGISTER_INTCAPA 0x10
#define REGISTER_INTCAPB 0x11
#define REGISTER_GPIOA   0x12
#define REGISTER_GPIOB   0x13
#define REGISTER_OLATA   0x14
#define REGISTER_OLATB   0x15

#define I2C_TIMEOUT    10
#define GPIOA_TOTAL_FB 7
#define GPIOB_TOTAL_FB 8

// Ports
#define MCP23017_PORTA 0x00
#define MCP23017_PORTB 0x01

/* SPI HARDWARE ADDRESS
** Defined by 7 bits, the first four bits are 0100, the last three are hardware
*defined in this order (A2, A1, A0)
** 0 1 0 0 A2 A1 A0
*/

// last three bits to 3V3
#define MCP23017_DEV1_ADDR 0x27
// last three bits to GND
#define MCP23017_DEV2_ADDR 0x20

// I/O Direction
// Default state: MCP23017_IODIR_ALL_INPUT
#define MCP23017_IODIR_ALL_OUTPUT 0x00
#define MCP23017_IODIR_ALL_INPUT  0xFF
#define MCP23017_IODIR_IO0_INPUT  0x01
#define MCP23017_IODIR_IO1_INPUT  0x02
#define MCP23017_IODIR_IO2_INPUT  0x04
#define MCP23017_IODIR_IO3_INPUT  0x08
#define MCP23017_IODIR_IO4_INPUT  0x10
#define MCP23017_IODIR_IO5_INPUT  0x20
#define MCP23017_IODIR_IO6_INPUT  0x40
#define MCP23017_IODIR_IO7_INPUT  0x80

// Input Polarity
// Default state: MCP23017_IPOL_ALL_NORMAL
#define MCP23017_IPOL_ALL_NORMAL   0x00
#define MCP23017_IPOL_ALL_INVERTED 0xFF
#define MCP23017_IPOL_IO0_INVERTED 0x01
#define MCP23017_IPOL_IO1_INVERTED 0x02
#define MCP23017_IPOL_IO2_INVERTED 0x04
#define MCP23017_IPOL_IO3_INVERTED 0x08
#define MCP23017_IPOL_IO4_INVERTED 0x10
#define MCP23017_IPOL_IO5_INVERTED 0x20
#define MCP23017_IPOL_IO6_INVERTED 0x40
#define MCP23017_IPOL_IO7_INVERTED 0x80

// Pull-Up Resistor
// Default state: MCP23017_GPPU_ALL_DISABLED
#define MCP23017_GPPU_ALL_DISABLED 0x00
#define MCP23017_GPPU_ALL_ENABLED  0xFF
#define MCP23017_GPPU_IO0_ENABLED  0x01
#define MCP23017_GPPU_IO1_ENABLED  0x02
#define MCP23017_GPPU_IO2_ENABLED  0x04
#define MCP23017_GPPU_IO3_ENABLED  0x08
#define MCP23017_GPPU_IO4_ENABLED  0x10
#define MCP23017_GPPU_IO5_ENABLED  0x20
#define MCP23017_GPPU_IO6_ENABLED  0x40
#define MCP23017_GPPU_IO7_ENABLED  0x80

typedef struct {
    I2C_HandleTypeDef *hi2c;
    uint16_t addr;
    uint8_t gpio[2];
} MCP23017_HandleTypeDef;

extern MCP23017_HandleTypeDef hmcp;

void mcp23017_init(MCP23017_HandleTypeDef *hdev);
HAL_StatusTypeDef mcp23017_read(MCP23017_HandleTypeDef *hdev, uint16_t reg, uint8_t *data);
HAL_StatusTypeDef mcp23017_write(MCP23017_HandleTypeDef *hdev, uint16_t reg, uint8_t *data);
HAL_StatusTypeDef mcp23017_iodir(MCP23017_HandleTypeDef *hdev, uint8_t port, uint8_t iodir);
HAL_StatusTypeDef mcp23017_ipol(MCP23017_HandleTypeDef *hdev, uint8_t port, uint8_t ipol);
HAL_StatusTypeDef mcp23017_ggpu(MCP23017_HandleTypeDef *hdev, uint8_t port, uint8_t pu);
HAL_StatusTypeDef mcp23017_read_gpio(MCP23017_HandleTypeDef *hdev, uint8_t port);
HAL_StatusTypeDef mcp23017_write_gpio(MCP23017_HandleTypeDef *hdev, uint8_t port);
uint8_t mcptest(MCP23017_HandleTypeDef *hdev);
#endif
