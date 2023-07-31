#include "main.h"
#include <stdio.h>

bool i2c_test_read_write_register() {
  uint8_t control[] = {0b10011111};

  HAL_I2C_Mem_Write(&hi2c4, ledaddr1, LED_CONTROL_REGISTER, 1, control, 1, 200);
  HAL_I2C_Mem_Write(&hi2c4, ledaddr2, LED_CONTROL_REGISTER, 1, control, 1, 200);

  uint8_t read_data[1] = {0};
  HAL_I2C_Mem_Read(&hi2c4, ledaddr1, LED_CONTROL_REGISTER, 1, read_data, 1,
                   200);
  if (read_data[0] != control[0]) {
#ifdef STEERING_LOG_ENABLED
    print("Not equal: %d != %d\n", read_data[0], control[0]);
#endif
    return false;
  }
  HAL_I2C_Mem_Read(&hi2c4, ledaddr2, LED_CONTROL_REGISTER, 1, read_data, 1,
                   200);
  if (read_data[0] != control[0]) {
#ifdef STEERING_LOG_ENABLED
    print("Not equal: %d != %d\n", read_data[0], control[0]);
#endif
    return false;
  }
  return true;
}
