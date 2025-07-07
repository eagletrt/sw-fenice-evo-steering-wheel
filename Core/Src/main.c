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
#include "fmc.h"
#include "gpio.h"
#include "i2c.h"
#include "ltdc.h"
#include "memorymap.h"
#include "octospi.h"
#include "tim.h"
#include "usart.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#define _XOPEN_SOURCE
#include <time.h>
#include "dma2d_utils.h"

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

#if CANSNIFFER_ENABLED == 1
extern cansniffer_elem_t primary_cansniffer_buffer[primary_MESSAGE_COUNT];
extern cansniffer_elem_t secondary_cansniffer_buffer[secondary_MESSAGE_COUNT];
#endif

extern bool primary_can_fatal_error;
extern bool secondary_can_fatal_error;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

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

    HAL_GPIO_WritePin(LCD_BL_EN_GPIO_Port, LCD_BL_EN_Pin, GPIO_PIN_SET);
    // HAL_GPIO_WritePin(LCD_BL_DIM_GPIO_Port, LCD_BL_DIM_Pin, GPIO_PIN_SET);
    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, 0);
    HAL_Delay(100);

    UI_t sw_screen;
    sw_init_screen(&sw_screen);

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

    uint32_t active_framebuffer = FRAMEBUFFER1_ADDR;
    uint32_t writable_framebuffer = FRAMEBUFFER2_ADDR;
    uint32_t last_swap_framebuffer = HAL_GetTick();
    static bool tson_button_pressed = false;
    static uint32_t tson_button_pressed_time_elapsed = 0;

    sw_set_canvas(&sw_screen, (uint32_t *)writable_framebuffer, SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_WIDTH);
    sw_screen.oc.pixels = (uint32_t *)writable_framebuffer;

    uint32_t start_poll = HAL_GetTick();
    while (start_poll + 1000 < HAL_GetTick() && !is_pmsg_new[primary_index_from_id(PRIMARY_ECU_POWER_MAPS_FRAME_ID)])
        ;
    float map_power = 1.0;
    int reg_state = 1;
    int sc_state = 1;
    int tv_state = 1;
    if (is_pmsg_new[primary_index_from_id(PRIMARY_ECU_POWER_MAPS_FRAME_ID)]) {
        GET_LAST_STATE(primary, ecu_power_maps, PRIMARY, ECU_POWER_MAPS);
        map_power = primary_ecu_power_maps_last_state->map_power;
        reg_state = primary_ecu_power_maps_last_state->reg_state;
        sc_state = primary_ecu_power_maps_last_state->sc_state;
        tv_state = primary_ecu_power_maps_last_state->tv_state;
    }
    GET_LAST_STATE(primary, ecu_set_power_maps, PRIMARY, ECU_SET_POWER_MAPS);
    primary_ecu_set_power_maps_last_state->map_power = map_power;
    primary_ecu_set_power_maps_last_state->reg_state = reg_state;
    primary_ecu_set_power_maps_last_state->sc_state = sc_state;
    primary_ecu_set_power_maps_last_state->tv_state = tv_state;

    GET_LAST_STATE(primary, steering_wheel_version, PRIMARY, STEERING_WHEEL_VERSION);
    struct tm timeinfo;
    strptime(__DATE__ " " __TIME__, "%b %d %Y %H:%M:%S", &timeinfo);
    primary_steering_wheel_version_last_state->canlib_build_time = CANLIB_BUILD_TIME;
    primary_steering_wheel_version_last_state->component_build_time = mktime(&timeinfo);

    /*
    GET_LAST_STATE(primary, hv_set_fans_status, PRIMARY, HV_SET_FANS_STATUS);
    primary_hv_set_fans_status_last_state->fans_override = primary_hv_set_fans_status_fans_override_off;
    primary_hv_set_fans_status_last_state->fans_speed    = 0.0f;
    */

    start_poll = HAL_GetTick();
    while (start_poll + 1000 < HAL_GetTick() && !is_pmsg_new[primary_index_from_id(PRIMARY_LV_PUMPS_SPEED_FRAME_ID)])
        ;
    int status = primary_lv_set_pumps_speed_status_manual;
    float pumps_speed = 0.0;
    if (is_pmsg_new[primary_index_from_id(PRIMARY_LV_PUMPS_SPEED_FRAME_ID)]) {
        GET_LAST_STATE(primary, lv_pumps_speed, PRIMARY, LV_PUMPS_SPEED);
        status = primary_lv_pumps_speed_last_state->status;
        pumps_speed = primary_lv_pumps_speed_last_state->pumps_speed;
    }
    GET_LAST_STATE(primary, lv_set_pumps_speed, PRIMARY, LV_SET_PUMPS_SPEED);
    primary_lv_set_pumps_speed_last_state->status = status;
    primary_lv_set_pumps_speed_last_state->pumps_speed = pumps_speed;

    start_poll = HAL_GetTick();
    while (start_poll + 1000 < HAL_GetTick() && !is_pmsg_new[primary_index_from_id(PRIMARY_LV_RADIATOR_SPEED_FRAME_ID)])
        ;
    status = primary_lv_set_radiator_speed_status_manual;
    float radiator_speed = 0.0;
    if (is_pmsg_new[primary_index_from_id(PRIMARY_LV_RADIATOR_SPEED_FRAME_ID)]) {
        GET_LAST_STATE(primary, lv_radiator_speed, PRIMARY, LV_RADIATOR_SPEED);
        status = primary_lv_radiator_speed_last_state->status;
        radiator_speed = primary_lv_radiator_speed_last_state->radiator_speed;
    }
    GET_LAST_STATE(primary, lv_set_radiator_speed, PRIMARY, LV_SET_RADIATOR_SPEED);
    primary_lv_set_radiator_speed_last_state->status = status;
    primary_lv_set_radiator_speed_last_state->radiator_speed = radiator_speed;

    /*
    GET_LAST_STATE(primary, lv_set_cooling_aggressiveness, PRIMARY, LV_SET_COOLING_AGGRESSIVENESS);
    primary_lv_set_cooling_aggressiveness_last_state->status = primary_lv_set_cooling_aggressiveness_status_normal;
    */

    GET_LAST_STATE(primary, ecu_set_ptt_status, PRIMARY, ECU_SET_PTT_STATUS);
    primary_ecu_set_ptt_status_last_state->status = primary_ecu_set_ptt_status_status_off;

    HAL_DMA2D_Init(&hdma2d);
    HAL_DMA2D_ConfigLayer(&hdma2d, DMA2D_BACKGROUND_LAYER);

    if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) {
        Error_Handler();
    }
    if (HAL_FDCAN_Start(&hfdcan2) != HAL_OK) {
        Error_Handler();
    }
    inputs_init();
#if WATCHDOG_ENABLED == 1
    init_watchdog();
#endif
#if CAN_OVER_SERIAL_ENABLED == 1
    can_over_serial_init();
#endif

    /* USER CODE END 2 */

    /* Infinite loop */
    /* USER CODE BEGIN WHILE */
    sw_screen_white(&sw_screen);

    while (1) {
#if CAN_OVER_SERIAL_ENABLED == 1
        can_over_serial_routine();
#endif
        static uint32_t last_read_inputs = 0;
        if ((get_current_time_ms() - last_read_inputs) > 10) {
            last_read_inputs = get_current_time_ms();
            read_inputs();
        }

        static uint32_t last_ptt_periodic_check = 0;
        if ((get_current_time_ms() - last_ptt_periodic_check) > 50) {
            last_ptt_periodic_check = get_current_time_ms();
            ptt_periodic_check(&sw_screen);
        }

        if ((get_current_time_ms() - last_swap_framebuffer) > 100) {
            last_swap_framebuffer = get_current_time_ms();
            extern int button_long_pressed;
            uint32_t button_lts = 0;

            if (button_long_pressed) {
                if (get_current_time_ms() - button_lts > 500) {
                    button_long_pressed = false;
                }
                sw_screen_white(&sw_screen);
            } else {
                sw_update_graphics_from_can_messages(&sw_screen);
                sw_update_screen(0.f, &sw_screen);
            }
            dma2d_m2m(writable_framebuffer, active_framebuffer, SCREEN_WIDTH, SCREEN_HEIGHT);
            // memcpy((uint8_t*) writable_framebuffer, (uint8_t*) active_framebuffer, SCREEN_WIDTH * SCREEN_HEIGHT * 4);
            uint32_t tmp = active_framebuffer;
            active_framebuffer = writable_framebuffer;
            writable_framebuffer = tmp;
            sw_screen.oc.pixels = (uint32_t *)writable_framebuffer;
            HAL_LTDC_SetAddress(&hltdc, active_framebuffer, LTDC_LAYER_1);
        }

        PERIODIC_SEND(primary, PRIMARY, ecu_set_power_maps, ECU_SET_POWER_MAPS);
        PERIODIC_SEND(primary, PRIMARY, steering_wheel_version, STEERING_WHEEL_VERSION);
        // PERIODIC_SEND(primary, PRIMARY, hv_set_fans_status, HV_SET_FANS_STATUS);
        PERIODIC_SEND(primary, PRIMARY, lv_set_pumps_speed, LV_SET_PUMPS_SPEED);
        PERIODIC_SEND(primary, PRIMARY, lv_set_radiator_speed, LV_SET_RADIATOR_SPEED);
        PERIODIC_SEND(primary, PRIMARY, ecu_set_ptt_status, ECU_SET_PTT_STATUS);
        // #define PRIMARY_INTERVAL_LV_SET_COOLING_AGGRESSIVENESS (1000U)
        // PERIODIC_SEND(primary, PRIMARY, lv_set_cooling_aggressiveness, LV_SET_COOLING_AGGRESSIVENESS);

        GPIO_PinState tson_pin_state = HAL_GPIO_ReadPin(TSON_BUTTON_GPIO_Port, TSON_BUTTON_Pin);

        if (tson_pin_state == GPIO_PIN_SET) {
            tson_button_pressed = false;
        } else if (!tson_button_pressed) {
            tson_button_pressed = true;
            tson_button_pressed_time_elapsed = get_current_time_ms();
        } else if ((get_current_time_ms() - tson_button_pressed_time_elapsed) > BUTTONS_LONG_PRESS_TIME) {
            prepare_and_send_ecu_set_status();
        }

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
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2 | RCC_CLOCKTYPE_D3PCLK1 |
                                  RCC_CLOCKTYPE_D1PCLK1;
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

void system_reset(void) {
    HAL_NVIC_SystemReset();
}

uint32_t get_current_time_ms(void) {
    return HAL_GetTick();
}

/* USER CODE END 4 */

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
