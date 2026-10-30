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
#include "dts.h"
#include "fdcan.h"
#include "fmac.h"
#include "i2c.h"
#include "usart.h"
#include "ltdc.h"
#include "octospi.h"
#include "tim.h"
#include "gpio.h"
#include "fmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "scheduler-api.h"
#include "can-communications-router-api.h"
#include "arena-allocator-api.h"
#include "pal-api.h"
#include "logger-api.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

#define LOGGER_ENABLED (true)          /*!< Logger status: true to enable active logging, false to mute entirely. */
#define LOGGER_RX_CAPACITY (1U)        /*!< Receive queue depth. Set to 1 because the logger is transmit-only but needs to be > 0 because of arena allocator. */
#define LOGGER_TX_CAPACITY (10U)       /*!< Maximum number of log message packets allowed to sit in the outbound transmission queue. */
#define LOGGER_UART_MAX_MSG_SIZE (64U) /*!< Maximum allocation allowed for an individual log string. */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define MAIN_CYCLES_PER_US_DIVIDER (1000000U) /*!< SystemCoreClock (Hz) to cycle-counter ticks per microsecond. */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MPU_Config(void);
/* USER CODE BEGIN PFP */

EAGLETRT_STATIC struct ArenaAllocatorHandler arena_allocator_handler;
EAGLETRT_STATIC struct PalHandler logger_pal_handler;

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

EAGLETRT_STATIC void prv_main_init_logging_configuration() {
    arena_allocator_api_init(&arena_allocator_handler);

    EAGLETRT_API_UNUSED(pal_api_init(&logger_pal_handler,
                                     LOGGER_RX_CAPACITY,
                                     LOGGER_TX_CAPACITY,
                                     LOGGER_UART_MAX_MSG_SIZE,
                                     NULL,
                                     usart_logger_transmit,
                                     NULL,
                                     NULL,
                                     &arena_allocator_handler));
}

/*!
 * \brief Sample the I2C expanders and the GPIO buttons (scheduler inputs task).
 *
 * \param tick Current tick in milliseconds.
 */
EAGLETRT_STATIC void prv_main_poll_inputs(uint32_t tick) {
    i2c_inputs_poll(tick);
    gpio_inputs_poll(tick);
}

/*!
 * \brief Free-running core cycle counter, used to time the scheduler tasks.
 */
EAGLETRT_STATIC uint32_t prv_main_get_cycles(void) {
    return DWT->CYCCNT;
}

/*!
 * \brief Log the DMA2D transfer timings of the last stats window, then reset them.
 *
 * \details Called by the scheduler stats task. Two short lines (the logger
 *     truncates long records): count, total and longest transfer per kind.
 */
EAGLETRT_STATIC void prv_main_report_stats(void) {
    const struct Dma2dOperationStats *stats = dma2d_get_stats();
    const struct Dma2dOperationStats *fill = &stats[DMA2D_OPERATION_FILL];
    const struct Dma2dOperationStats *blend = &stats[DMA2D_OPERATION_BLEND];
    const struct Dma2dOperationStats *mask = &stats[DMA2D_OPERATION_MASK];
    const struct Dma2dOperationStats *copy = &stats[DMA2D_OPERATION_COPY];
    EAGLETRT_API_UNUSED(logger_api_log(LOGGER_LEVEL_INFO, "d2d fill %lu/%luus/%lu blend %lu/%luus/%lu", fill->count, fill->total_us, fill->max_us, blend->count, blend->total_us, blend->max_us));
    EAGLETRT_API_UNUSED(logger_api_log(LOGGER_LEVEL_INFO, "d2d mask %lu/%luus/%lu copy %lu/%luus/%lu", mask->count, mask->total_us, mask->max_us, copy->count, copy->total_us, copy->max_us));
    dma2d_reset_stats();

    // Bus-off restarts (fdcan.c): logged only when one happened, the UART is slow.
    static uint32_t reported_bus_off[CAN_COMMUNICATION_NETWORK_COUNT];
    const uint32_t primary_bus_off = fdcan_get_bus_off_count(CAN_COMMUNICATION_NETWORK_PRIMARY);
    const uint32_t secondary_bus_off = fdcan_get_bus_off_count(CAN_COMMUNICATION_NETWORK_SECONDARY);
    if (primary_bus_off != reported_bus_off[CAN_COMMUNICATION_NETWORK_PRIMARY] || secondary_bus_off != reported_bus_off[CAN_COMMUNICATION_NETWORK_SECONDARY]) {
        reported_bus_off[CAN_COMMUNICATION_NETWORK_PRIMARY] = primary_bus_off;
        reported_bus_off[CAN_COMMUNICATION_NETWORK_SECONDARY] = secondary_bus_off;
        EAGLETRT_API_UNUSED(logger_api_log(LOGGER_LEVEL_WARN, "can bus-off restarts p %lu s %lu", primary_bus_off, secondary_bus_off));
    }
}

/*!
 * \brief Log why the MCU last reset, then clear the flags.
 *
 * \details Neither the bootloader nor the HAL clears RCC_RSR, so the flags
 *     still describe the reset that started the bootloader. SFT is also a
 *     reset asked over CAN or by Error_Handler (which logs before it).
 */
EAGLETRT_STATIC void prv_main_log_reset_cause(void) {
    const uint32_t flags = RCC->RSR;
    __HAL_RCC_CLEAR_RESET_FLAGS();
    EAGLETRT_API_UNUSED(logger_api_log(LOGGER_LEVEL_INFO,
                                       "reset 0x%08lX%s%s%s%s%s",
                                       flags,
                                       ((flags & RCC_RSR_PORRSTF) != 0U) ? " POR" : "",
                                       ((flags & RCC_RSR_BORRSTF) != 0U) ? " BOR" : "",
                                       ((flags & RCC_RSR_PINRSTF) != 0U) ? " PIN" : "",
                                       ((flags & RCC_RSR_SFTRSTF) != 0U) ? " SFT" : "",
                                       ((flags & (RCC_RSR_IWDG1RSTF | RCC_RSR_WWDG1RSTF)) != 0U) ? " WDG" : ""));
}

/*!
 * \brief Start the DWT cycle counter.
 */
EAGLETRT_STATIC void prv_main_enable_cycle_counter(void) {
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->LAR = 0xC5ACCE55U; // Cortex-M7: unlock the DWT registers
    DWT->CYCCNT = 0U;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

// bootloader code
static void VectorBase_Config(void) {
    /* The constant array with vectors of the vector table is declared externally in the
     * c-startup code.
     */
    extern const unsigned long g_pfnVectors[];

    /* Remap the vector table to where the vector table is located for this program. */
    SCB->VTOR = (unsigned long)&g_pfnVectors[0];
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {

    /* USER CODE BEGIN 1 */
    VectorBase_Config(); // bootloader code
    /* USER CODE END 1 */

    /* MPU Configuration--------------------------------------------------------*/
    MPU_Config();

    /* Enable the CPU Cache */

    /* Enable I-Cache---------------------------------------------------------*/
    SCB_EnableICache();

    /* Enable D-Cache---------------------------------------------------------*/
    SCB_EnableDCache();

    /* MCU Configuration--------------------------------------------------------*/

    /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
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
    MX_DTS_Init();
    MX_FMAC_Init();
    /* USER CODE BEGIN 2 */

    prv_main_init_logging_configuration();
    EAGLETRT_API_UNUSED(logger_api_init(&logger_pal_handler, LOGGER_ENABLED));
    prv_main_log_reset_cause();

    HAL_GPIO_WritePin(LCD_BL_EN_GPIO_Port, LCD_BL_EN_Pin, GPIO_PIN_SET);
    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0);
    HAL_Delay(100);

    fdcan_start();

    i2c_inputs_init();

    prv_main_enable_cycle_counter();

    const struct SchedulerConfig scheduler_config = {
        .post = {
            .can_network_configs = {
                [CAN_COMMUNICATION_NETWORK_PRIMARY] = {
                    .cs_enter = __disable_irq,
                    .cs_exit = __enable_irq,
                    .on_receive = can_communications_router_api_receive_primary,
                    .send = fdcan_send_primary,
                },
                [CAN_COMMUNICATION_NETWORK_SECONDARY] = {
                    .cs_enter = __disable_irq,
                    .cs_exit = __enable_irq,
                    .on_receive = can_communications_router_api_receive_secondary,
                    .send = fdcan_send_secondary,
                },
            },
            .draw_rectangle = ltdc_draw_rectangle,
            .screen_accelerators = {
                .draw_mask = ltdc_draw_mask,
                .mask_buffer = ltdc_mask_buffer(),
                .mask_size = ltdc_mask_size(),
            },
        },
        .get_tick = HAL_GetTick,
        .poll_inputs = prv_main_poll_inputs,
        .swap_framebuffers = ltdc_swap_framebuffers,
        .reset = NVIC_SystemReset,
        .get_cycles = prv_main_get_cycles,
        .cycles_per_us = SystemCoreClock / MAIN_CYCLES_PER_US_DIVIDER,
        .report_stats = prv_main_report_stats,
    };

    if (scheduler_api_init(&scheduler_config) != SCHEDULER_RC_OK) {
        Error_Handler();
    }

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        EAGLETRT_API_UNUSED(scheduler_api_routine());
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
    RCC_OscInitTypeDef RCC_OscInitStruct = { 0 };
    RCC_ClkInitTypeDef RCC_ClkInitStruct = { 0 };

    /** Supply configuration update enable
  */
    HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

    /** Configure the main internal regulator output voltage
  */
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE0);

    while (!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {
    }

    /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_CSI | RCC_OSCILLATORTYPE_HSI | RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
    RCC_OscInitStruct.HSICalibrationValue = 64;
    RCC_OscInitStruct.CSIState = RCC_CSI_ON;
    RCC_OscInitStruct.CSICalibrationValue = 16;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 6;
    RCC_OscInitStruct.PLL.PLLN = 137;
    RCC_OscInitStruct.PLL.PLLP = 1;
    RCC_OscInitStruct.PLL.PLLQ = 5;
    RCC_OscInitStruct.PLL.PLLR = 2;
    RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
    RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
    RCC_OscInitStruct.PLL.PLLFRACN = 4096;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
  */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1 | RCC_CLOCKTYPE_D1PCLK1;
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

/* MPU Configuration */

void MPU_Config(void) {
    MPU_Region_InitTypeDef MPU_InitStruct = { 0 };

    /* Disables the MPU */
    HAL_MPU_Disable();

    /** Initializes and configures the Region and the memory to be protected
  */
    MPU_InitStruct.Enable = MPU_REGION_ENABLE;
    MPU_InitStruct.Number = MPU_REGION_NUMBER0;
    MPU_InitStruct.BaseAddress = 0xC0000000;
    MPU_InitStruct.Size = MPU_REGION_SIZE_8MB;
    MPU_InitStruct.SubRegionDisable = 0x0;
    MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL1;
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
    MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsBufferable = MPU_ACCESS_BUFFERABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);
    /* Enables the MPU */
    HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM4 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    /* USER CODE BEGIN Callback 0 */

    /* USER CODE END Callback 0 */
    if (htim->Instance == TIM4) {
        HAL_IncTick();
    }
    /* USER CODE BEGIN Callback 1 */

    /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void) {
    /* USER CODE BEGIN Error_Handler_Debug */
    /* Log who failed, then reset instead of hanging: a transient fault gets
     * another try through the bootloader, which also keeps the wheel
     * flashable. The address goes through addr2line with firmware.elf. A
     * failure before the logger is initialized is not logged (the call just
     * returns). */
    __disable_irq();
    const uintptr_t caller = (uintptr_t)__builtin_return_address(0);
    EAGLETRT_API_UNUSED(logger_api_log(LOGGER_LEVEL_ERROR, "Error_Handler from 0x%08lX, resetting", (unsigned long)caller));
    NVIC_SystemReset();
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
