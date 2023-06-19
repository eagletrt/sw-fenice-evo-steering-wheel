/**
 * @file mcp23017.c
 * @author Tommaso Canova [tommaso.canova@studenti.unitn.it]
 * @brief
 * @version 0.1
 * @date 2022-05-04
 *
 * @copyright Copyright (c) 2022
 *
 */

#include "inputs/mcp23017.h"

#include "main.h"
#include "stdio.h"
#include <string.h>

MCP23017_HandleTypeDef hmcp;

HAL_StatusTypeDef mcp23017_read(MCP23017_HandleTypeDef *hdev, uint16_t reg,
                                uint8_t *data) {
  return HAL_I2C_Mem_Read(hdev->hi2c, hdev->addr, reg, 1, data, 1, I2C_TIMEOUT);
}

HAL_StatusTypeDef mcp23017_write(MCP23017_HandleTypeDef *hdev, uint16_t reg,
                                 uint8_t *data) {
  return HAL_I2C_Mem_Write(hdev->hi2c, hdev->addr, reg, 1, data, 1,
                           I2C_TIMEOUT);
}

HAL_StatusTypeDef mcp23017_iodir(MCP23017_HandleTypeDef *hdev, uint8_t port,
                                 uint8_t iodir) {
  uint8_t data[1] = {iodir};
  return mcp23017_write(hdev, REGISTER_IODIRA | port, data);
}

HAL_StatusTypeDef mcp23017_ipol(MCP23017_HandleTypeDef *hdev, uint8_t port,
                                uint8_t ipol) {
  uint8_t data[1] = {ipol};
  return mcp23017_write(hdev, REGISTER_IPOLA | port, data);
}

HAL_StatusTypeDef mcp23017_ggpu(MCP23017_HandleTypeDef *hdev, uint8_t port,
                                uint8_t pu) {
  uint8_t data[1] = {pu};
  return mcp23017_write(hdev, REGISTER_GPPUA | port, data);
}

HAL_StatusTypeDef mcp23017_read_gpio(MCP23017_HandleTypeDef *hdev,
                                     uint8_t port) {
  uint8_t data[1];
  HAL_StatusTypeDef status;
  status = mcp23017_read(hdev, REGISTER_GPIOA | port, data);
  if (status == HAL_OK)
    hdev->gpio[port] = data[0];
  return status;
}

HAL_StatusTypeDef mcp23017_write_gpio(MCP23017_HandleTypeDef *hdev,
                                      uint8_t port) {
  uint8_t data = {hdev->gpio[port]};
  return mcp23017_write(hdev, REGISTER_GPIOA | port, &data);
}

/**
 * @brief Necessary and tested configuration to initialize correctly the
 * MCP23017 Using default address 0x20
 *
 * @param hdev MCP23017_HandleTypeDef handle for the device
 */
void mcp23017_init(MCP23017_HandleTypeDef *hdev) {
  mcp23017_iodir(hdev, MCP23017_PORTA, MCP23017_IODIR_ALL_INPUT);
  mcp23017_iodir(hdev, MCP23017_PORTB, MCP23017_IODIR_ALL_INPUT);
  mcp23017_ggpu(hdev, MCP23017_PORTA, MCP23017_GPPU_ALL_DISABLED);
  mcp23017_ggpu(hdev, MCP23017_PORTB, MCP23017_GPPU_ALL_DISABLED);
  mcp23017_ipol(hdev, MCP23017_PORTA, MCP23017_IPOL_ALL_NORMAL);
  mcp23017_ipol(hdev, MCP23017_PORTB, MCP23017_IPOL_ALL_NORMAL);
}

uint8_t mcp23017_get_state(MCP23017_HandleTypeDef *hdev, uint8_t gpio_port,
                           uint8_t gpio_pin) {
  gpio_pin %= 8;
  return (hdev->gpio[gpio_port] & (1 << gpio_pin)) >> gpio_pin;
}

/**
 * @brief Test function of Read/Write registers
 *
 * @param hdev MCP23017_HandleTypeDef handle for the device
 * @return 1 if all registers have passed the test
 * @return 0 if something has gone wrong
 */
uint8_t mcptest(MCP23017_HandleTypeDef *hdev) {
  for (uint16_t reg = 0; reg < REGISTER_INTFA; reg++) {
    if (reg != 10 && reg != 11) {
      uint8_t data = 0;
      uint8_t reg_data;
      mcp23017_read(hdev, reg, &reg_data);
      for (uint8_t bit = 0; bit < 8; bit++) {
        data = (1 << bit);
        mcp23017_write(hdev, reg, &data);
        mcp23017_read(hdev, reg, &data);
        if (data != (1 << bit)) {
          data = 0xFF;
          return 1;
        }
      }
      mcp23017_write(hdev, reg, &reg_data);
    }
  }

  mcp23017_iodir(hdev, 0, 0);
  mcp23017_iodir(hdev, 1, 0);
  for (uint16_t reg = REGISTER_GPIOA; reg < REGISTER_OLATB + 1; reg++) {
    uint8_t data = 0;
    uint8_t reg_data;
    mcp23017_read(hdev, reg, &reg_data);
    for (uint8_t bit = 0; bit < 8; bit++) {
      data = (1 << bit);
      mcp23017_write(hdev, reg, &data);
      mcp23017_read(hdev, reg, &data);
      if (data != (1 << bit)) {
        data = 0xFF;
        return 0;
      }
    }
    mcp23017_write(hdev, reg, &reg_data);
  }
  mcp23017_iodir(hdev, 0, 0xFF);
  mcp23017_iodir(hdev, 1, 0xFF);
  return 1;
}
