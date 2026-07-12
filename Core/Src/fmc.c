/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : FMC.c
 * Description        : This file provides code for the configuration
 *                      of the FMC peripheral.
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
#include "fmc.h"

/* USER CODE BEGIN 0 */

#include "eagletrt-api.h"
#include "micron-steer-sdram-api.h"

/*!
 * \brief SDRAM refresh counter for this board's FMC clock.
 *
 * \details Differs from MICRON_STEER_SDRAM_REFRESH_COUNT because the old
 *     wheel runs the FMC at a different clock than the new one; the value
 *     is carried over from the previous firmware.
 */
#define FMC_MICRON_STEER_SDRAM_REFRESH_COUNT (0x0603U)

/*!
 * \brief Delay callback for the SDRAM driver, wrapping HAL_Delay.
 */
static void prv_fmc_micron_steer_sdram_delay(uint32_t delay_ms) {
    HAL_Delay(delay_ms);
}

/*!
 * \brief Send-command callback for the SDRAM driver, wrapping HAL_SDRAM_SendCommand.
 */
static enum MicronSteerSdramReturnCode prv_fmc_micron_steer_send_command(struct MicronSteerSdramCommand *cmd) {
    if (cmd == NULL) {
        return MICRON_STEER_SDRAM_RC_ERROR;
    }

    FMC_SDRAM_CommandTypeDef command = {
        .CommandMode = cmd->command_mode,
        .CommandTarget = cmd->target_bank,
        .AutoRefreshNumber = cmd->auto_refresh_number,
        .ModeRegisterDefinition = cmd->mode_register_definition
    };

    if (HAL_SDRAM_SendCommand(&hsdram1, &command, MICRON_STEER_SDRAM_TIMEOUT) != HAL_OK) {
        return MICRON_STEER_SDRAM_RC_ERROR;
    }

    return MICRON_STEER_SDRAM_RC_OK;
}

/*!
 * \brief Refresh-rate callback for the SDRAM driver, wrapping HAL_SDRAM_ProgramRefreshRate.
 */
static enum MicronSteerSdramReturnCode prv_fmc_micron_steer_program_refresh_rate(uint32_t refresh_rate) {
    if (HAL_SDRAM_ProgramRefreshRate(&hsdram1, refresh_rate) != HAL_OK) {
        return MICRON_STEER_SDRAM_RC_ERROR;
    }

    return MICRON_STEER_SDRAM_RC_OK;
}

/* USER CODE END 0 */

SDRAM_HandleTypeDef hsdram1;

/* FMC initialization function */
void MX_FMC_Init(void) {
    /* USER CODE BEGIN FMC_Init 0 */

    /* USER CODE END FMC_Init 0 */

    FMC_SDRAM_TimingTypeDef SdramTiming = {0};

    /* USER CODE BEGIN FMC_Init 1 */

    /* USER CODE END FMC_Init 1 */

    /** Perform the SDRAM1 memory initialization sequence
  */
    hsdram1.Instance = FMC_SDRAM_DEVICE;
    /* hsdram1.Init */
    hsdram1.Init.SDBank             = FMC_SDRAM_BANK1;
    hsdram1.Init.ColumnBitsNumber   = FMC_SDRAM_COLUMN_BITS_NUM_8;
    hsdram1.Init.RowBitsNumber      = FMC_SDRAM_ROW_BITS_NUM_12;
    hsdram1.Init.MemoryDataWidth    = FMC_SDRAM_MEM_BUS_WIDTH_16;
    hsdram1.Init.InternalBankNumber = FMC_SDRAM_INTERN_BANKS_NUM_4;
    hsdram1.Init.CASLatency         = FMC_SDRAM_CAS_LATENCY_3;
    hsdram1.Init.WriteProtection    = FMC_SDRAM_WRITE_PROTECTION_DISABLE;
    hsdram1.Init.SDClockPeriod      = FMC_SDRAM_CLOCK_PERIOD_2;
    hsdram1.Init.ReadBurst          = FMC_SDRAM_RBURST_ENABLE;
    hsdram1.Init.ReadPipeDelay      = FMC_SDRAM_RPIPE_DELAY_0;
    /* SdramTiming */
    SdramTiming.LoadToActiveDelay    = 2;
    SdramTiming.ExitSelfRefreshDelay = 7;
    SdramTiming.SelfRefreshTime      = 4;
    SdramTiming.RowCycleDelay        = 6;
    SdramTiming.WriteRecoveryTime    = 2;
    SdramTiming.RPDelay              = 2;
    SdramTiming.RCDDelay             = 2;

    if (HAL_SDRAM_Init(&hsdram1, &SdramTiming) != HAL_OK) {
        Error_Handler();
    }

    /* USER CODE BEGIN FMC_Init 2 */

    // these commands before the actual init are used to recover from a self-refresh or power-down state, which can happen if the SDRAM is not properly initialized after a reset
    struct MicronSteerSdramCommand recovery_cmd = {
        .command_mode = MICRON_STEER_SDRAM_CLK_ENABLE_CMD,
        .target_bank = FMC_SDRAM_CMD_TARGET_BANK1,
        .auto_refresh_number = 1,
        .mode_register_definition = 0,
    };
    prv_fmc_micron_steer_send_command(&recovery_cmd); /* CKE high: exits self-refresh/power-down */
    HAL_Delay(1);                                     /* covers tXSR (67 ns) with huge margin     */

    recovery_cmd.command_mode = MICRON_STEER_SDRAM_PALL_CMD;
    prv_fmc_micron_steer_send_command(&recovery_cmd); /* close any row left open mid-burst        */

    recovery_cmd.command_mode = MICRON_STEER_SDRAM_AUTOREFRESH_MODE_CMD;
    recovery_cmd.auto_refresh_number = 8;
    prv_fmc_micron_steer_send_command(&recovery_cmd); /* re-stabilise the internal refresh state  */

    struct MicronSteerSdramContext micron_steer_sdram_ctx = {
        .target_bank = FMC_SDRAM_CMD_TARGET_BANK1,
        .refresh_mode = MICRON_STEER_SDRAM_AUTOREFRESH_MODE_CMD,
        .refresh_rate = FMC_MICRON_STEER_SDRAM_REFRESH_COUNT,
        .burst_length = MICRON_STEER_SDRAM_BURST_LENGTH_1,
        .burst_type = MICRON_STEER_SDRAM_BURST_TYPE_SEQUENTIAL,
        .cas_latency = MICRON_STEER_SDRAM_CAS_LATENCY_3,
        .operation_mode = MICRON_STEER_SDRAM_OPERATING_MODE_STANDARD,
        .write_burst_mode = MICRON_STEER_SDRAM_WRITEBURST_MODE_SINGLE,
    };

    if (micron_steer_sdram_api_init(&micron_steer_sdram_ctx, prv_fmc_micron_steer_sdram_delay, prv_fmc_micron_steer_send_command, prv_fmc_micron_steer_program_refresh_rate) != MICRON_STEER_SDRAM_RC_OK) {
        Error_Handler();
    }

    /* USER CODE END FMC_Init 2 */
}

static uint32_t FMC_Initialized = 0;

static void HAL_FMC_MspInit(void) {
    /* USER CODE BEGIN FMC_MspInit 0 */

    /* USER CODE END FMC_MspInit 0 */
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (FMC_Initialized) {
        return;
    }
    FMC_Initialized                              = 1;
    RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

    /** Initializes the peripherals clock
  */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FMC;
    PeriphClkInitStruct.PLL2.PLL2M           = 2;
    PeriphClkInitStruct.PLL2.PLL2N           = 50;
    PeriphClkInitStruct.PLL2.PLL2P           = 2;
    PeriphClkInitStruct.PLL2.PLL2Q           = 4;
    PeriphClkInitStruct.PLL2.PLL2R           = 4;
    PeriphClkInitStruct.PLL2.PLL2RGE         = RCC_PLL2VCIRANGE_3;
    PeriphClkInitStruct.PLL2.PLL2VCOSEL      = RCC_PLL2VCOWIDE;
    PeriphClkInitStruct.PLL2.PLL2FRACN       = 0;
    PeriphClkInitStruct.FmcClockSelection    = RCC_FMCCLKSOURCE_PLL2;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /* Peripheral clock enable */
    __HAL_RCC_FMC_CLK_ENABLE();

    /** FMC GPIO Configuration
  PF0   ------> FMC_A0
  PF1   ------> FMC_A1
  PF2   ------> FMC_A2
  PF3   ------> FMC_A3
  PF4   ------> FMC_A4
  PF5   ------> FMC_A5
  PC0   ------> FMC_SDNWE
  PC2_C   ------> FMC_SDNE0
  PC3_C   ------> FMC_SDCKE0
  PF11   ------> FMC_SDNRAS
  PF12   ------> FMC_A6
  PF13   ------> FMC_A7
  PF14   ------> FMC_A8
  PF15   ------> FMC_A9
  PG0   ------> FMC_A10
  PG1   ------> FMC_A11
  PE7   ------> FMC_D4
  PE8   ------> FMC_D5
  PE9   ------> FMC_D6
  PE10   ------> FMC_D7
  PE11   ------> FMC_D8
  PE12   ------> FMC_D9
  PE13   ------> FMC_D10
  PE14   ------> FMC_D11
  PE15   ------> FMC_D12
  PD8   ------> FMC_D13
  PD9   ------> FMC_D14
  PD10   ------> FMC_D15
  PD14   ------> FMC_D0
  PD15   ------> FMC_D1
  PG4   ------> FMC_BA0
  PG5   ------> FMC_BA1
  PG8   ------> FMC_SDCLK
  PD0   ------> FMC_D2
  PD1   ------> FMC_D3
  PG15   ------> FMC_SDNCAS
  PE0   ------> FMC_NBL0
  PE1   ------> FMC_NBL1
  */
    /* GPIO_InitStruct */
    GPIO_InitStruct.Pin  = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 |
                           GPIO_PIN_15;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

    HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

    /* GPIO_InitStruct */
    GPIO_InitStruct.Pin       = GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    /* GPIO_InitStruct */
    GPIO_InitStruct.Pin       = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_15;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

    HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

    /* GPIO_InitStruct */
    GPIO_InitStruct.Pin       = GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15 |
                                GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    /* GPIO_InitStruct */
    GPIO_InitStruct.Pin       = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15 | GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_NOPULL;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF12_FMC;

    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    /* USER CODE BEGIN FMC_MspInit 1 */

    /* USER CODE END FMC_MspInit 1 */
}

void HAL_SDRAM_MspInit(SDRAM_HandleTypeDef *sdramHandle) {
    /* USER CODE BEGIN SDRAM_MspInit 0 */

    EAGLETRT_API_UNUSED(sdramHandle);

    /* USER CODE END SDRAM_MspInit 0 */
    HAL_FMC_MspInit();
    /* USER CODE BEGIN SDRAM_MspInit 1 */

    /* USER CODE END SDRAM_MspInit 1 */
}

static uint32_t FMC_DeInitialized = 0;

static void HAL_FMC_MspDeInit(void) {
    /* USER CODE BEGIN FMC_MspDeInit 0 */

    /* USER CODE END FMC_MspDeInit 0 */
    if (FMC_DeInitialized) {
        return;
    }
    FMC_DeInitialized = 1;
    /* Peripheral clock enable */
    __HAL_RCC_FMC_CLK_DISABLE();

    /** FMC GPIO Configuration
  PF0   ------> FMC_A0
  PF1   ------> FMC_A1
  PF2   ------> FMC_A2
  PF3   ------> FMC_A3
  PF4   ------> FMC_A4
  PF5   ------> FMC_A5
  PC0   ------> FMC_SDNWE
  PC2_C   ------> FMC_SDNE0
  PC3_C   ------> FMC_SDCKE0
  PF11   ------> FMC_SDNRAS
  PF12   ------> FMC_A6
  PF13   ------> FMC_A7
  PF14   ------> FMC_A8
  PF15   ------> FMC_A9
  PG0   ------> FMC_A10
  PG1   ------> FMC_A11
  PE7   ------> FMC_D4
  PE8   ------> FMC_D5
  PE9   ------> FMC_D6
  PE10   ------> FMC_D7
  PE11   ------> FMC_D8
  PE12   ------> FMC_D9
  PE13   ------> FMC_D10
  PE14   ------> FMC_D11
  PE15   ------> FMC_D12
  PD8   ------> FMC_D13
  PD9   ------> FMC_D14
  PD10   ------> FMC_D15
  PD14   ------> FMC_D0
  PD15   ------> FMC_D1
  PG4   ------> FMC_BA0
  PG5   ------> FMC_BA1
  PG8   ------> FMC_SDCLK
  PD0   ------> FMC_D2
  PD1   ------> FMC_D3
  PG15   ------> FMC_SDNCAS
  PE0   ------> FMC_NBL0
  PE1   ------> FMC_NBL1
  */

    HAL_GPIO_DeInit(
        GPIOF,
        GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15);

    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_0 | GPIO_PIN_2 | GPIO_PIN_3);

    HAL_GPIO_DeInit(GPIOG, GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_8 | GPIO_PIN_15);

    HAL_GPIO_DeInit(
        GPIOE,
        GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 | GPIO_PIN_14 | GPIO_PIN_15 | GPIO_PIN_0 | GPIO_PIN_1);

    HAL_GPIO_DeInit(GPIOD, GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_14 | GPIO_PIN_15 | GPIO_PIN_0 | GPIO_PIN_1);

    /* USER CODE BEGIN FMC_MspDeInit 1 */

    /* USER CODE END FMC_MspDeInit 1 */
}

void HAL_SDRAM_MspDeInit(SDRAM_HandleTypeDef *sdramHandle) {
    /* USER CODE BEGIN SDRAM_MspDeInit 0 */

    EAGLETRT_API_UNUSED(sdramHandle);

    /* USER CODE END SDRAM_MspDeInit 0 */
    HAL_FMC_MspDeInit();
    /* USER CODE BEGIN SDRAM_MspDeInit 1 */

    /* USER CODE END SDRAM_MspDeInit 1 */
}
/**
  * @}
  */

/**
  * @}
  */
