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

#include "fsm.h"
#include "parameters-api.h"
#include "post.h"
#include "screen-api.h"
#include "ui-data-api.h"
#include "can-communications-router-api.h"
#include "identity-api.h"
#include "can-primary.h"
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

#define MAIN_INPUTS_POLL_PERIOD_MS (10U)

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

/*!
 * \brief Mirror a parameter transition into the UI snapshot.
 *
 * \param parameter_id Parameter that changed.
 * \param value New value of the parameter.
 */
EAGLETRT_STATIC void prv_main_sync_parameter_to_ui(enum InputsSharedParameterID parameter_id, uint8_t value) {
    struct UIData *ui_data = ui_data_api_get();
    switch (parameter_id) {
        case INPUTS_SHARED_PARAMETER_ID_POWER:
            ui_data->power = value;
            break;
        case INPUTS_SHARED_PARAMETER_ID_REGEN:
            ui_data->regen = value;
            break;
        case INPUTS_SHARED_PARAMETER_ID_TORQUE_VECTORING:
            ui_data->torque = value;
            break;
        case INPUTS_SHARED_PARAMETER_ID_LAUNCH_CONTROL:
            ui_data->slip_on = value;
            break;
        case INPUTS_SHARED_PARAMETER_ID_PTT:
            ui_data->ptt = value;
            break;
        default:
            break;
    }
}

/*!
 * \brief Map the wheel FSM state onto the status signal of SteeringWheelFsm.
 *
 * \param state The current FSM state.
 *
 * \return The matching CanPrimarySteeringwheelfsmStatus value.
 */
EAGLETRT_STATIC enum CanPrimarySteeringwheelfsmStatus prv_main_fsm_status(fsm_state_t state) {
    switch (state) {
        case FSM_STATE_IDLE:
            return CAN_PRIMARY_STEERINGWHEELFSM_STATUS_IDLE;
        case FSM_STATE_ERROR:
            return CAN_PRIMARY_STEERINGWHEELFSM_STATUS_ERROR;
        case FSM_STATE_FLASH:
            return CAN_PRIMARY_STEERINGWHEELFSM_STATUS_FLASH;
        case FSM_STATE_AUTONOMOUS:
            return CAN_PRIMARY_STEERINGWHEELFSM_STATUS_AUTONOMOUS;
        case FSM_STATE_INIT:
        default:
            return CAN_PRIMARY_STEERINGWHEELFSM_STATUS_INIT;
    }
}

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
 * \brief Queue every cyclic frame the wheel owes the bus this tick.
 *
 * \details Called just before the FSM runs, so anything queued here is
 *     drained by the process_tx pass inside the same iteration.
 *
 * \param state The current FSM state, broadcast as our identity status.
 * \param tick Current tick in milliseconds.
 */
EAGLETRT_STATIC void prv_main_broadcast_can(fsm_state_t state, uint32_t tick) {
    EAGLETRT_API_UNUSED(can_communications_router_api_process_tx_periodic(tick));
    EAGLETRT_API_UNUSED(identity_api_periodically_send_state(prv_main_fsm_status(state), tick));
    EAGLETRT_API_UNUSED(identity_api_periodically_send_version(tick));
    EAGLETRT_API_UNUSED(identity_api_periodically_send_libcan_version(tick));
}

bool main_on_parameter_change(enum InputsSharedParameterID parameter_id, uint8_t value) {
    // Every transition goes on the bus, shared with the UI or not: TS-on
    // and PTT are exactly the two that never reach the popup.
    if (can_communications_router_api_on_parameter_change(parameter_id, value) != CAN_COMMUNICATION_RC_OK) {
        return false;
    }

    // Every parameter reaches the dashboard; parameters_api_is_shared only
    // decides which ones additionally raise the popup. PTT is the case that
    // matters: it never pops up, but it does tint the SCENARIO header.
    prv_main_sync_parameter_to_ui(parameter_id, value);

    if (!parameters_api_is_shared(parameter_id)) {
        return true;
    }

    return screen_api_on_parameter_change(parameter_id, value) == SCREEN_RC_OK;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void) {

    /* USER CODE BEGIN 1 */

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

    fsm_state_t current_state = FSM_STATE_INIT;

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

    HAL_GPIO_WritePin(LCD_BL_EN_GPIO_Port, LCD_BL_EN_Pin, GPIO_PIN_SET);
    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0);
    HAL_Delay(100);

    fdcan_start();

    i2c_inputs_init();

    struct PostInitData post_init_data = {
        .parameters_on_change = main_on_parameter_change,
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
    };

    current_state = fsm_run_state(current_state, &post_init_data);

    struct FsmData fsm_data = { 0 };
    fsm_data.swap_framebuffers = ltdc_swap_framebuffers;
    fsm_data.reset = NVIC_SystemReset;
    uint32_t last_inputs_poll_tick = 0U;

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    while (1) {
        fsm_data.tick = HAL_GetTick();

        if (fsm_data.tick - last_inputs_poll_tick >= MAIN_INPUTS_POLL_PERIOD_MS) {
            last_inputs_poll_tick = fsm_data.tick;
            i2c_inputs_poll(fsm_data.tick);
            gpio_inputs_poll(fsm_data.tick);
        }

        prv_main_broadcast_can(current_state, fsm_data.tick);

        current_state = fsm_run_state(current_state, &fsm_data);
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
