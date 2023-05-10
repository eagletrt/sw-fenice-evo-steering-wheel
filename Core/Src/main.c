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
enum framebuffer { FRAMEBUFFER1, FRAMEBUFFER2 };

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

static enum framebuffer active = FRAMEBUFFER1;

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
  led_control_set_all(&hi2c4, COLOR_RED);
#endif

#define SDRAM_TESTS 0
#if SDRAM_TESTS == 1
  sdram_test_write_all();
  sdram_test_segments();
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

  /*
  For some reasons goes HardFault...
  uint8_t *** display_buffer = (uint8_t ***) SDRAM_BASE_ADDRESS;
  for (uint32_t icol = 0; icol < SCREEN_HEIGHT; icol++) {
    for (uint32_t irow = 0; irow < SCREEN_WIDTH; irow++) {
      display_buffer[icol][irow][0] = 0x30;
      display_buffer[icol][irow][1] = 0xFF;
      display_buffer[icol][irow][2] = 0x1C;
    }
  }
  */

  lv_init();
  screen_driver_init();
  lv_example_grid_4();
  // tab_manager();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    // LV_UPDATE_PROPERTY(ESTIMATED_VELOCITY, steering.AMBIENT_TEMPERATURE++);

#if 1
    HAL_StatusTypeDef retval =
        HAL_UART_Transmit(&hlpuart1, (uint8_t *)"Hello\n", 6, 100);
    if (retval != HAL_OK) {
      Error_Handler();
    }
#endif

    HAL_Delay(2000);

    lv_tasks();

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
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSI);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType =
      RCC_OSCILLATORTYPE_CSI | RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.CSIState = RCC_CSI_ON;
  RCC_OscInitStruct.CSICalibrationValue = 16;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 34;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 3072;
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

void lv_example_grid_4(void) {
  static lv_coord_t col_dsc[] = {60, 60, 60, LV_GRID_TEMPLATE_LAST};
  static lv_coord_t row_dsc[] = {45, 45, 45, LV_GRID_TEMPLATE_LAST};

  /*Add space between the columns and move the rows to the bottom (end)*/

  /*Create a container with grid*/
  lv_obj_t *cont = lv_obj_create(lv_scr_act());
  lv_obj_set_grid_align(cont, LV_GRID_ALIGN_SPACE_BETWEEN, LV_GRID_ALIGN_END);
  lv_obj_set_grid_dsc_array(cont, col_dsc, row_dsc);
  lv_obj_set_size(cont, 300, 220);
  lv_obj_center(cont);

  lv_obj_t *label;
  lv_obj_t *obj;
  uint32_t i;
  for (i = 0; i < 9; i++) {
    uint8_t col = i % 3;
    uint8_t row = i / 3;

    obj = lv_obj_create(cont);
    /*Stretch the cell horizontally and vertically too
     *Set span to 1 to make the cell 1 column/row sized*/
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, col, 1,
                         LV_GRID_ALIGN_STRETCH, row, 1);

    label = lv_label_create(obj);
    lv_label_set_text_fmt(label, "%d,%d", col, row);
    lv_obj_center(label);
  }
}

void LTDC_switch_framebuffer(void) {
  if (active == FRAMEBUFFER1) {
    LTDC_Layer1->CFBAR = FRAMEBUFFER2_ADDR;
    active = FRAMEBUFFER2;
  } else {
    LTDC_Layer1->CFBAR = FRAMEBUFFER1_ADDR;
    active = FRAMEBUFFER1;
  }
  LTDC->SRCR = LTDC_SRCR_VBR; // reload shadow registers on vertical blank
  while ((LTDC->CDSR & LTDC_CDSR_VSYNCS) == 0) // wait for reload
    ;
}

uint32_t *LTDC_get_backbuffer_address(void) {
  if (active == FRAMEBUFFER1)
    return (uint32_t *)FRAMEBUFFER2_ADDR;
  else
    return (uint32_t *)FRAMEBUFFER1_ADDR;
}

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
