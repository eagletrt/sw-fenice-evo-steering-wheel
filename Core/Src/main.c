/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2023 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dac.h"
#include "dma2d.h"
#include "fdcan.h"
#include "fmc.h"
#include "gpio.h"
#include "i2c.h"
#include "ltdc.h"
#include "octospi.h"
#include "tim.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

void lv_example_grid_4(void);
void LTDC_switch_framebuffer(void);
uint32_t *LTDC_get_backbuffer_address(void);
lv_color_t *framebuffer_1 = (lv_color_t *)FRAMEBUFFER1_ADDR;
lv_color_t *framebuffer_2 = (lv_color_t *)FRAMEBUFFER2_ADDR;

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick.
   */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_FDCAN1_Init();
  MX_FDCAN2_Init();
  MX_FMC_Init();
  MX_LPUART1_UART_Init();
  MX_LTDC_Init();
  MX_OCTOSPI1_Init();
  MX_I2C4_Init();
  MX_TIM5_Init();
  MX_DAC1_Init();
  MX_TIM7_Init();
  MX_DMA2D_Init();
  /* USER CODE BEGIN 2 */

  HAL_GPIO_WritePin(LCD_BL_EN_GPIO_Port, LCD_BL_EN_Pin, GPIO_PIN_SET);
  HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 4096);
  HAL_Delay(100);

#if 1
  led_control_init();
  led_control_set_all(&hi2c4, COLOR_BLUE);
#endif

#define SDRAM_TESTS 0
#if SDRAM_TESTS == 1
  sdram_test_write_all();
  sdram_test_long_arrays();
  sdram_test_end_of_memory();
  sdram_test_simple_write();
#endif

#if 0 // Green screen
  uint8_t *display_buffer = (uint8_t *)SDRAM_BASE_ADDRESS;
  for (uint32_t icell = 0; icell < SCREEN_HEIGHT * SCREEN_WIDTH; ++icell) {
    display_buffer[4 * icell] = 0xFF;
    display_buffer[4 * icell + 1] = 0xFF;
    display_buffer[4 * icell + 2] = 0x00;
    display_buffer[4 * icell + 3] = 0x00;
  }
#endif

#define SCREEN_ENABLED 1
#if SCREEN_ENABLED == 1
  lv_init();
  screen_driver_init();
  tab_manager();
#endif

#if 0

  uint8_t data[TSIZE];
  memcpy(data, (void *)EXTERNAL_FLASH_ADDRESS, TSIZE);
  data[TSIZE - 1] = '\0';
  print("roba: %s\n", (char *)data);
#endif

  HAL_TIM_Base_Start_IT(&htim7);

#define EXTERNAL_FLASH_ENABLED 0
#if EXTERNAL_FLASH_ENABLED == 1

#define EXTERNAL_FLASH_ADDRESS 0x90000000
#define TSIZE 20
  uint32_t command = 0;
  uint32_t address = 0;

  OSPI_RegularCmdTypeDef ospi_command = {
      .OperationType = HAL_OSPI_OPTYPE_READ_CFG,
      .FlashId = HAL_OSPI_FLASH_ID_1,
      .Instruction = command,
      .InstructionMode = HAL_OSPI_INSTRUCTION_1_LINE,
      .InstructionSize = HAL_OSPI_INSTRUCTION_8_BITS,
      .InstructionDtrMode = HAL_OSPI_INSTRUCTION_DTR_DISABLE,
      .Address = address,
      .AddressMode = HAL_OSPI_ADDRESS_1_LINE,
      .AddressSize = HAL_OSPI_ADDRESS_8_BITS,
      .AddressDtrMode = HAL_OSPI_ADDRESS_DTR_DISABLE,
      .AlternateBytes = 0,
      .AlternateBytesMode = HAL_OSPI_ALTERNATE_BYTES_NONE,
      .AlternateBytesSize = HAL_OSPI_ALTERNATE_BYTES_8_BITS, // ignored
      .AlternateBytesDtrMode = HAL_OSPI_ALTERNATE_BYTES_DTR_DISABLE,
      .DataMode = HAL_OSPI_DATA_DTR_DISABLE,
      .NbData = 0, // ?????
      .DataDtrMode = HAL_OSPI_DATA_DTR_DISABLE,
      .DummyCycles = 0,
      .DQSMode = HAL_OSPI_DQS_DISABLE,
      .SIOOMode = HAL_OSPI_SIOO_INST_EVERY_CMD,
  };
  HAL_OSPI_Command(&hospi1, &ospi_command, 1000);
// HAL_OSPI_Command(&hospi1);
#endif

#if 0
  int8_t data[100] = {};
  uint32_t val = HAL_OSPI_GetState(&hospi1);
  print("HAL_OSPI_GetState = %u\n", (unsigned int)val);
  HAL_StatusTypeDef status = HAL_OSPI_Receive(&hospi1, data, 500);
  if (status == HAL_OK) {
    print("HAL OK\n");
  } else {
    print("HAL NOT OK\n");
  }
#endif

  lv_timer_t *steer_status_task =
      lv_timer_create(send_steer_status, PRIMARY_INTERVAL_STEER_STATUS, NULL);
  lv_timer_set_repeat_count(steer_status_task, -1);
  lv_timer_reset(steer_status_task);

  lv_timer_t *steer_version_task =
      lv_timer_create(send_steer_version, PRIMARY_INTERVAL_STEER_VERSION, NULL);
  lv_timer_set_repeat_count(steer_version_task, -1);
  lv_timer_reset(steer_version_task);

  lv_timer_t *read_inputs_task = lv_timer_create(read_inputs, 100, NULL);
  lv_timer_set_repeat_count(read_inputs_task, -1);
  lv_timer_reset(read_inputs_task);

#if 0
  can_message_t msg = {0};
  primary_set_car_status_t car_status = {0};
  car_status.car_status_set = primary_set_car_status_car_status_set_READY;
  msg.id = PRIMARY_SET_CAR_STATUS_FRAME_ID;
  msg.size = PRIMARY_SET_CAR_STATUS_BYTE_SIZE;
  primary_set_car_status_pack(msg.data, &car_status,
                              PRIMARY_SET_CAR_STATUS_BYTE_SIZE);
  can_send(&msg, &hfdcan1);
#endif

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1) {

#if SCREEN_ENABLED == 1
    lv_tasks();
#endif

#if 0
    HAL_StatusTypeDef retval =
        HAL_UART_Transmit(&hlpuart1, (uint8_t *)"Hello\n", 6, 100);
    if (retval != HAL_OK) {
      Error_Handler();
    }
#endif
#if 0
    can_message_t msg;
    msg.id = 0x2;
    msg.size = 4;
    msg.data[0] = 3;
    msg.data[1] = 4;
    msg.data[2] = 5;
    msg.data[3] = 6;
    print("Sending CAN message with id %" PRIu32 "\n", msg.id);
    can_send(&msg, &hfdcan1);
    HAL_Delay(1000);
#endif

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Supply configuration update enable
   */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
   */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
  }

  /** Macro to configure the PLL clock source
   */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSE);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType =
      RCC_OSCILLATORTYPE_CSI | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.CSIState = RCC_CSI_ON;
  RCC_OscInitStruct.CSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 3;
  RCC_OscInitStruct.PLL.PLLN = 62;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 5;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 4096;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
   */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 |
                                RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK) {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1) {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(uint8_t *file, uint32_t line) {
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line
     number, ex: printf("Wrong parameters value: file %s on line %d\r\n", file,
     line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
