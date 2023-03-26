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
#define WIDTH SCREEN_WIDTH
#define HEIGHT SCREEN_HEIGHT
#define BACKGROUND_COLOR 0xFF181818
#define CIRCLE_RADIUS 100
#define CIRCLE_COLOR 0x99AA2020
#define PI 3.14159265359

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static enum framebuffer active = FRAMEBUFFER1;

static float triangle_angle = 0;
static float circle_x = WIDTH / 2;
static float circle_y = HEIGHT / 2;
static float circle_dx = 100;
static float circle_dy = 100;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

float sqrtf(float x);
float atan2f(float y, float x);
float sinf(float x);
float cosf(float x);

static inline void rotate_point(float *x, float *y);
Olivec_Canvas vc_render(float dt);
void LTDC_switch_framebuffer(void);
uint32_t *LTDC_get_backbuffer_address(void);

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

#define SDRAM_TESTS 1
#if SDRAM_TESTS
  // sdram_test1();
  // sdram_test2();
  // sdram_test3();
  sdram_test4();
  sdram_test5();
#endif

  led_control_init();
  led_control_set_all(&hi2c4, COLOR_OFF);

  /*
  Japan Flag
  uint32_t *pixels = (uint32_t *)SDRAM_BASE_ADDRESS;
  Olivec_Canvas oc =
      olivec_canvas(pixels, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH);

  olivec_fill(oc, 0xFFFFFFFF);
  olivec_circle(oc, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, 180, 0xFFFF0000);

    Green screen
    for (uint32_t icell = 0; icell < SCREEN_HEIGHT * SCREEN_WIDTH; ++icell) {
      display_buffer[3 * icell] = 0x00;
      display_buffer[3 * icell + 1] = 0xFF;
      display_buffer[3 * icell + 2] = 0x00;
    }

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

  HAL_Delay(100);
  HAL_GPIO_WritePin(LCD_BL_EN_GPIO_Port, LCD_BL_EN_Pin, GPIO_PIN_SET);
  HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 4096);

  uint32_t prev = HAL_GetTick();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1) {
    uint32_t curr = HAL_GetTick();
    float dt = (curr - prev) / 10000.0f;
    prev = curr;
    LTDC_switch_framebuffer();
    vc_render(dt);

    HAL_Delay(16);
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
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
  }

  /** Macro to configure the PLL clock source
   */
  __HAL_RCC_PLL_PLLSOURCE_CONFIG(RCC_PLLSOURCE_HSI);

  /** Initializes the RCC Oscillators according to the specified parameters
   * in the RCC_OscInitTypeDef structure.
   */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 18;
  RCC_OscInitStruct.PLL.PLLP = 1;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_3;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 6144;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

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

static inline void rotate_point(float *x, float *y) {
  float dx = *x - WIDTH / 2;
  float dy = *y - HEIGHT / 2;
  float mag = sqrtf(dx * dx + dy * dy);
  float dir = atan2f(dy, dx) + triangle_angle;
  *x = cosf(dir) * mag + WIDTH / 2;
  *y = sinf(dir) * mag + HEIGHT / 2;
}

Olivec_Canvas vc_render(float dt) {
  uint32_t *pixels = LTDC_get_backbuffer_address();
  Olivec_Canvas oc = olivec_canvas(pixels, WIDTH, HEIGHT, WIDTH);

  olivec_fill(oc, BACKGROUND_COLOR);

  // Triangle
  {
    triangle_angle += 0.5f * PI * dt;

    float x1 = WIDTH / 2, y1 = HEIGHT / 8;
    float x2 = WIDTH / 8, y2 = HEIGHT / 2;
    float x3 = WIDTH * 7 / 8, y3 = HEIGHT * 7 / 8;
    rotate_point(&x1, &y1);
    rotate_point(&x2, &y2);
    rotate_point(&x3, &y3);
    olivec_triangle3c(oc, x1, y1, x2, y2, x3, y3, 0xFF2020FF, 0xFF20FF20,
                      0xFFFF2020);
  }

  // Circle
  {
    float x = circle_x + circle_dx * dt;
    if (x - CIRCLE_RADIUS < 0 || x + CIRCLE_RADIUS >= WIDTH) {
      circle_dx *= -1;
    } else {
      circle_x = x;
    }

    float y = circle_y + circle_dy * dt;
    if (y - CIRCLE_RADIUS < 0 || y + CIRCLE_RADIUS >= HEIGHT) {
      circle_dy *= -1;
    } else {
      circle_y = y;
    }

    olivec_circle(oc, circle_x, circle_y, CIRCLE_RADIUS, CIRCLE_COLOR);
  }

  return oc;
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
