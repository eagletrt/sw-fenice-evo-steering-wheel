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

primary_watchdog m_primary_watchdog = {0};
secondary_watchdog m_secondary_watchdog = {0};

can_id_t primary_watchdog_monitored_ids[] = PRIMARY_MONITORED_MESSAGES;
const size_t primary_watchdog_monitored_ids_size =
    sizeof(primary_watchdog_monitored_ids) / sizeof(can_id_t);

can_id_t secondary_watchdog_monitored_ids[] = SECONDARY_MONITORED_MESSAGES;
const size_t secondary_watchdog_monitored_ids_size =
    sizeof(secondary_watchdog_monitored_ids) / sizeof(can_id_t);

lv_color_t *framebuffer_1 = (lv_color_t *)FRAMEBUFFER1_ADDR;
lv_color_t *framebuffer_2 = (lv_color_t *)FRAMEBUFFER2_ADDR;

#if CANSNIFFER_ENABLED == 1
cansniffer_elem_t init_pbuf[CAN_POSSIBLE_IDS];
cansniffer_elem_t init_sbuf[CAN_POSSIBLE_IDS];
/***
 * Cansniffer buffers in the external SDRAM are too slow:
 * they need to be in the internal one.
 */
cansniffer_elem_t *primary_cansniffer_buffer = (cansniffer_elem_t *)
    init_pbuf; // (cansniffer_elem_t* ) PRIMARY_CANSNIFFER_MEMORY_POOL_ADDRESS;
cansniffer_elem_t *secondary_cansniffer_buffer = (cansniffer_elem_t *)
    init_sbuf; // (cansniffer_elem_t*) SECONDARY_CANSNIFFER_MEMORY_POOL_ADDRESS;
#endif

#if DEBUG_RX_BUFFERS_ENABLED == 1
extern uint32_t debug_rx_counters[4];

void update_rx_fn(lv_timer_t *tim) {
  char buffer[128];
  sprintf(buffer,
          "{ sec_rx1 = %lu, sec_rx0 = %lu, prim_rx1 = %lu, prim_rx0 = %lu }",
          debug_rx_counters[0], debug_rx_counters[1], debug_rx_counters[2],
          debug_rx_counters[3]);
  tab_terminal_new_message(buffer);
}
#endif

extern bool primary_can_fatal_error;
extern bool secondary_can_fatal_error;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void watchdog_task_fn(lv_timer_t *);
void ptt_tasks_fn(lv_timer_t *tim);

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
  MX_TIM7_Init();
  MX_DMA2D_Init();
  MX_DAC1_Init();
  /* USER CODE BEGIN 2 */

  HAL_GPIO_WritePin(LCD_BL_EN_GPIO_Port, LCD_BL_EN_Pin, GPIO_PIN_SET);
  // HAL_GPIO_WritePin(LCD_BL_DIM_GPIO_Port, LCD_BL_DIM_Pin, GPIO_PIN_SET);
  HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0);
  HAL_Delay(100);

#if 1
  led_control_init();
  led_control_set_all(&hi2c4, COLOR_BLUE);
#endif

#define I2C_TESTS 0
#if I2C_TESTS == 1
  i2c_test_read_write_register();
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

#if CANSNIFFER_ENABLED == 1
  cansniffer_buffer_init();
#endif

#define SCREEN_ENABLED 1
#if SCREEN_ENABLED == 1
  lv_init();
  screen_driver_init();
  tab_manager();
#endif

  HAL_TIM_Base_Start_IT(&htim7);

  for (uint64_t iindex = 0; iindex < primary_watchdog_monitored_ids_size;
       ++iindex) {
    can_id_t id = primary_watchdog_monitored_ids[iindex];
    CANLIB_BITSET_ARRAY(m_primary_watchdog.activated,
                        primary_watchdog_index_from_id(id));
  }
  for (uint64_t iindex = 0; iindex < secondary_watchdog_monitored_ids_size;
       ++iindex) {
    can_id_t id = secondary_watchdog_monitored_ids[iindex];
    CANLIB_BITSET_ARRAY(m_secondary_watchdog.activated,
                        secondary_watchdog_index_from_id(id));
  }

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

  lv_timer_t *shutdown_circuit_task =
      lv_timer_create(update_shutdown_circuit_ui, 100, NULL);
  lv_timer_set_repeat_count(shutdown_circuit_task, -1);
  lv_timer_reset(shutdown_circuit_task);

  lv_timer_t *ptt_tasks = lv_timer_create(ptt_tasks_fn, 1000, NULL);
  lv_timer_set_repeat_count(ptt_tasks, -1);
  lv_timer_reset(ptt_tasks);

#if DEBUG_RX_BUFFERS_ENABLED == 1
  lv_timer_t *update_rx_task = lv_timer_create(update_rx_fn, 2000, NULL);
  lv_timer_set_repeat_count(update_rx_task, -1);
  lv_timer_reset(update_rx_task);
#endif

  if (primary_can_fatal_error) {
    enter_fatal_error_mode("Primary CAN fatal error");
    Error_Handler();
  }
  if (secondary_can_fatal_error) {
    enter_fatal_error_mode("Secondary CAN fatal error");
    Error_Handler();
  }

  // lv_timer_t *watchdog_task = lv_timer_create(watchdog_task_fn, 5000, NULL);
  // lv_timer_set_repeat_count(watchdog_task, -1);
  // lv_timer_reset(watchdog_task);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  while (1) {

#if SCREEN_ENABLED == 1
    lv_tasks();
#endif

    changed_pin_fn();

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

void openblt_reset(void) {
#ifdef STEERING_LOG_ENABLED
  print("Resetting for open blt\n");
#endif
  HAL_NVIC_SystemReset();
}

uint32_t get_current_time_ms(void) { return HAL_GetTick(); }

void watchdog_task_fn(lv_timer_t *main_timer) {
  UNUSED(main_timer);

  primary_watchdog_timeout(&m_primary_watchdog, HAL_GetTick());
  secondary_watchdog_timeout(&m_secondary_watchdog, HAL_GetTick());

  for (uint64_t iindex = 0; iindex < primary_watchdog_monitored_ids_size;
       ++iindex) {
    can_id_t id = primary_watchdog_monitored_ids[iindex];
    bool timed_out = CANLIB_BITTEST_ARRAY(m_primary_watchdog.timeout,
                                          primary_watchdog_index_from_id(id));
    if (timed_out) {
      char name[128];
      primary_message_name_from_id(id, name);
      switch (id) {
      case PRIMARY_CAR_STATUS_FRAME_ID:
        break;
      }
    }
  }
  for (uint64_t iindex = 0; iindex < secondary_watchdog_monitored_ids_size;
       ++iindex) {
    can_id_t id = secondary_watchdog_monitored_ids[iindex];
    bool timed_out = CANLIB_BITTEST_ARRAY(m_secondary_watchdog.timeout,
                                          secondary_watchdog_index_from_id(id));
    if (timed_out) {
      char name[128];
      secondary_message_name_from_id(id, name);
    }
  }
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
