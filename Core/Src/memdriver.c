/**
******************************************************************************
* @file    mt48lc4m32b2.c
* @author  MCD Application Team
* @author  Giacomo Mazzucchi
* @brief   MT48LC4M16A2 sdram 64Mbit driver file
******************************************************************************
* @attention
*
* <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
* All rights reserved.</center></h2>
*
* This software component is licensed by ST under BSD 3-Clause license,
* the "License"; You may not use this #include <stdint.h>
file except in compliance with the
* License. You may obtain a copy of the License at:
*                        opensource.org/licenses/BSD-3-Clause
*
******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "memdriver.h"

/** @addtogroup BSP
 * @{
 */

/** @addtogroup Components
 * @{
 */

/** @defgroup MT48LC4M16A2 MICRON_STEER
 * @brief     This file provides a set of functions needed to drive the
 *            MT48LC4M16A2 SDRAM memory.
 * @{
 */

/** @defgroup MICRON_STEER_Private_Variables MICRON_STEER Private Variables
 * @{
 */
static FMC_SDRAM_CommandTypeDef Command;
/**
 * @}
 */

/** @defgroup MICRON_STEER_Function_Prototypes MICRON_STEER Function Prototypes
 * @{
 */
static int32_t MICRON_STEER_Delay(uint32_t Delay);

/**
 * @}
 */

/** @defgroup MicronSteer_Exported_Functions MICRON_STEER Exported Functions
 * @{
 */
/**
 * @brief  Initializes the MT48LC4M16A2 SDRAM memory
 * @param  Ctx : Component object pointer
 * @param  pRegMode : Pointer to Register Mode stucture
 * @retval error status
 */
int32_t MicronSteer_Init(SDRAM_HandleTypeDef *Ctx, MICRON_STEER_Context_t *pRegMode) {
    int32_t ret = MICRON_STEER_ERROR;

    /* Step 1: Configure a clock configuration enable command */
    if (MicronSteer_ClockEnable(Ctx, pRegMode->TargetBank) == MICRON_STEER_OK) {
        /* Step 2: Insert 100 us minimum delay */
        /* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
        (void)MICRON_STEER_Delay(1);

        /* Step 3: Configure a PALL (precharge all) command */
        if (MicronSteer_Precharge(Ctx, pRegMode->TargetBank) == MICRON_STEER_OK) {
            /* Step 4: Configure a Refresh command */
            if (MicronSteer_RefreshMode(Ctx, pRegMode->TargetBank, pRegMode->RefreshMode) == MICRON_STEER_OK) {
                /* Step 5: Program the external memory mode register */
                if (MicronSteer_ModeRegConfig(Ctx, pRegMode) == MICRON_STEER_OK) {
                    /* Step 6: Set the refresh rate counter */
                    if (MicronSteer_RefreshRate(Ctx, pRegMode->RefreshRate) == MICRON_STEER_OK) {
                        ret = MICRON_STEER_OK;
                    }
                }
            }
        }
    }
    return ret;
}

/**
 * @brief  Enable SDRAM clock
 * @param  Ctx : Component object pointer
 * @param  Interface : Could be FMC_SDRAM_CMD_TARGET_BANK1 or
 * FMC_SDRAM_CMD_TARGET_BANK2
 * @retval error status
 */
int32_t MicronSteer_ClockEnable(SDRAM_HandleTypeDef *Ctx, uint32_t Interface) {
    Command.CommandMode            = MICRON_STEER_CLK_ENABLE_CMD;
    Command.CommandTarget          = Interface;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    if (HAL_SDRAM_SendCommand(Ctx, &Command, MICRON_STEER_TIMEOUT) != HAL_OK) {
        return MICRON_STEER_ERROR;
    } else {
        return MICRON_STEER_OK;
    }
}

/**
 * @brief  Precharge all sdram banks
 * @param  Ctx : Component object pointer
 * @param  Interface : Could be FMC_SDRAM_CMD_TARGET_BANK1 or
 * FMC_SDRAM_CMD_TARGET_BANK2
 * @retval error status
 */
int32_t MicronSteer_Precharge(SDRAM_HandleTypeDef *Ctx, uint32_t Interface) {
    Command.CommandMode            = MICRON_STEER_PALL_CMD;
    Command.CommandTarget          = Interface;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    if (HAL_SDRAM_SendCommand(Ctx, &Command, MICRON_STEER_TIMEOUT) != HAL_OK) {
        return MICRON_STEER_ERROR;
    } else {
        return MICRON_STEER_OK;
    }
}

/**
 * @brief  Program the external memory mode register
 * @param  Ctx : Component object pointer
 * @param  pRegMode : Pointer to Register Mode stucture
 * @retval error status
 */
int32_t MicronSteer_ModeRegConfig(SDRAM_HandleTypeDef *Ctx, MICRON_STEER_Context_t *pRegMode) {
    uint32_t tmpmrd;

    /* Program the external memory mode register */
    tmpmrd = (uint32_t)pRegMode->BurstLength | pRegMode->BurstType | pRegMode->CASLatency | pRegMode->OperationMode | pRegMode->WriteBurstMode;

    Command.CommandMode            = MICRON_STEER_LOAD_MODE_CMD;
    Command.CommandTarget          = pRegMode->TargetBank;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = tmpmrd;

    /* Send the command */
    if (HAL_SDRAM_SendCommand(Ctx, &Command, MICRON_STEER_TIMEOUT) != HAL_OK) {
        return MICRON_STEER_ERROR;
    } else {
        return MICRON_STEER_OK;
    }
}

/**
 * @brief  Program the SDRAM timing
 * @param  Ctx : Component object pointer
 * @param  pTiming : Pointer to SDRAM timing configuration stucture
 * @retval error status
 */
int32_t MicronSteer_TimingConfig(SDRAM_HandleTypeDef *Ctx, FMC_SDRAM_TimingTypeDef *pTiming) {
    /* Program the SDRAM timing */
    if (HAL_SDRAM_Init(Ctx, pTiming) != HAL_OK) {
        return MICRON_STEER_ERROR;
    } else {
        return MICRON_STEER_OK;
    }
}

/**
 * @brief  Configure Refresh mode
 * @param  Ctx : Component object pointer
 * @param  Interface : Could be FMC_SDRAM_CMD_TARGET_BANK1 or
 * FMC_SDRAM_CMD_TARGET_BANK2
 * @param  RefreshMode : Could be MICRON_STEER_CMD_AUTOREFRESH_MODE or
 *                      MICRON_STEER_CMD_SELFREFRESH_MODE
 * @retval error status
 */
int32_t MicronSteer_RefreshMode(SDRAM_HandleTypeDef *Ctx, uint32_t Interface, uint32_t RefreshMode) {
    Command.CommandMode            = RefreshMode;
    Command.CommandTarget          = Interface;
    Command.AutoRefreshNumber      = 8;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    if (HAL_SDRAM_SendCommand(Ctx, &Command, MICRON_STEER_TIMEOUT) != HAL_OK) {
        return MICRON_STEER_ERROR;
    } else {
        return MICRON_STEER_OK;
    }
}

/**
 * @brief  Set the device refresh rate
 * @param  Ctx : Component object pointer
 * @param  RefreshCount : The refresh rate to be programmed
 * @retval error status
 */
int32_t MicronSteer_RefreshRate(SDRAM_HandleTypeDef *Ctx, uint32_t RefreshCount) {
    /* Set the device refresh rate */
    if (HAL_SDRAM_ProgramRefreshRate(Ctx, RefreshCount) != HAL_OK) {
        return MICRON_STEER_ERROR;
    } else {
        return MICRON_STEER_OK;
    }
}

/**
 * @brief  Enter Power mode
 * @param  Ctx : Component object pointer
 * @param  Interface : Could be FMC_SDRAM_CMD_TARGET_BANK1 or
 * FMC_SDRAM_CMD_TARGET_BANK2
 * @retval error status
 */
int32_t MicronSteer_EnterPowerMode(SDRAM_HandleTypeDef *Ctx, uint32_t Interface) {
    Command.CommandMode            = MICRON_STEER_POWERDOWN_MODE_CMD;
    Command.CommandTarget          = Interface;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    if (HAL_SDRAM_SendCommand(Ctx, &Command, MICRON_STEER_TIMEOUT) != HAL_OK) {
        return MICRON_STEER_ERROR;
    } else {
        return MICRON_STEER_OK;
    }
}

/**
 * @brief  Exit Power mode
 * @param  Ctx : Component object pointer
 * @param  Interface : Could be FMC_SDRAM_CMD_TARGET_BANK1 or
 * FMC_SDRAM_CMD_TARGET_BANK2
 * @retval error status
 */
int32_t MicronSteer_ExitPowerMode(SDRAM_HandleTypeDef *Ctx, uint32_t Interface) {
    Command.CommandMode            = MICRON_STEER_NORMAL_MODE_CMD;
    Command.CommandTarget          = Interface;
    Command.AutoRefreshNumber      = 1;
    Command.ModeRegisterDefinition = 0;

    /* Send the command */
    if (HAL_SDRAM_SendCommand(Ctx, &Command, MICRON_STEER_TIMEOUT) != HAL_OK) {
        return MICRON_STEER_ERROR;
    } else {
        return MICRON_STEER_OK;
    }
}

/**
 * @brief  Sends command to the SDRAM bank.
 * @param  Ctx : Component object pointer
 * @param  SdramCmd : Pointer to SDRAM command structure
 * @retval SDRAM status
 */
int32_t MicronSteer_Sendcmd(SDRAM_HandleTypeDef *Ctx, FMC_SDRAM_CommandTypeDef *SdramCmd) {
    if (HAL_SDRAM_SendCommand(Ctx, SdramCmd, MICRON_STEER_TIMEOUT) != HAL_OK) {
        return MICRON_STEER_ERROR;
    } else {
        return MICRON_STEER_OK;
    }
}

/**
 * @}
 */

/** @defgroup MICRON_STEER_Private_Functions MICRON_STEER Private Functions
 * @{
 */

/**
 * @brief This function provides accurate delay (in milliseconds)
 * @param Delay: specifies the delay time length, in milliseconds
 * @retval MICRON_STEER_OK
 */
static int32_t MICRON_STEER_Delay(uint32_t Delay) {
    uint32_t tickstart;
    tickstart = HAL_GetTick();
    while ((HAL_GetTick() - tickstart) < Delay) {
    }
    return MICRON_STEER_OK;
}

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
