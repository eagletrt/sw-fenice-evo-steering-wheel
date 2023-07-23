/**
 ******************************************************************************
 * @file    memdriver.h
 * @author  MCD Application Team
 * @author  Giacomo Mazzucchi
 * @brief   This file contains all the description of the MICRON MT48LC4M16A2 SDRAM
 *          memory.
 *
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef MICRON_STEER_H
#define MICRON_STEER_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/

/** @addtogroup BSP
 * @{
 */

/** @addtogroup Components
 * @{
 */

/** @addtogroup MICRON_STEER
 * @{
 */

#include "main.h"

#define REFRESH_COUNT ((uint32_t)0x0603) /* SDRAM refresh counter */

#define MICRON_STEER_TIMEOUT ((uint32_t)0xFFFF)

/** @defgroup MicronSteer_Exported_Types MICRON_STEER Exported Types
 * @{
 */
typedef struct {
  uint32_t TargetBank;     /*!< Target Bank                             */
  uint32_t RefreshMode;    /*!< Refresh Mode                            */
  uint32_t RefreshRate;    /*!< Refresh Rate                            */
  uint32_t BurstLength;    /*!< Burst Length                            */
  uint32_t BurstType;      /*!< Burst Type                              */
  uint32_t CASLatency;     /*!< CAS Latency                             */
  uint32_t OperationMode;  /*!< Operation Mode                          */
  uint32_t WriteBurstMode; /*!< Write Burst Mode                        */
} MICRON_STEER_Context_t;

/**
 * @}
 */

/** @defgroup MicronSteer_Exported_Constants MICRON_STEER Exported Constants
 * @{
 */
#define MICRON_STEER_OK (0)
#define MICRON_STEER_ERROR (-1)

/* Register Mode */
#define MICRON_STEER_BURST_LENGTH_1 0x00000000U
#define MICRON_STEER_BURST_LENGTH_2 0x00000001U
#define MICRON_STEER_BURST_LENGTH_4 0x00000002U
#define MICRON_STEER_BURST_LENGTH_8 0x00000004U
#define MICRON_STEER_BURST_TYPE_SEQUENTIAL 0x00000000U
#define MICRON_STEER_BURST_TYPE_INTERLEAVED 0x00000008U
#define MICRON_STEER_CAS_LATENCY_2 0x00000020U
#define MICRON_STEER_CAS_LATENCY_3 0x00000030U
#define MICRON_STEER_OPERATING_MODE_STANDARD 0x00000000U
#define MICRON_STEER_WRITEBURST_MODE_PROGRAMMED 0x00000000U
#define MICRON_STEER_WRITEBURST_MODE_SINGLE 0x00000200U

/* Command Mode */
#define MICRON_STEER_NORMAL_MODE_CMD 0x00000000U
#define MICRON_STEER_CLK_ENABLE_CMD 0x00000001U
#define MICRON_STEER_PALL_CMD 0x00000002U
#define MICRON_STEER_AUTOREFRESH_MODE_CMD 0x00000003U
#define MICRON_STEER_LOAD_MODE_CMD 0x00000004U
#define MICRON_STEER_SELFREFRESH_MODE_CMD 0x00000005U
#define MICRON_STEER_POWERDOWN_MODE_CMD 0x00000006U

/**
 * @}
 */

/** @addtogroup MicronSteer_Exported_Functions
 * @{
 */
int32_t MicronSteer_Init(SDRAM_HandleTypeDef *Ctx,
                          MICRON_STEER_Context_t *pRegMode);
int32_t MicronSteer_ClockEnable(SDRAM_HandleTypeDef *Ctx, uint32_t Interface);
int32_t MicronSteer_Precharge(SDRAM_HandleTypeDef *Ctx, uint32_t Interface);
int32_t MicronSteer_ModeRegConfig(SDRAM_HandleTypeDef *Ctx,
                                   MICRON_STEER_Context_t *pRegMode);
int32_t MicronSteer_TimingConfig(SDRAM_HandleTypeDef *Ctx,
                                  FMC_SDRAM_TimingTypeDef *pTiming);
int32_t MicronSteer_RefreshMode(SDRAM_HandleTypeDef *Ctx, uint32_t Interface,
                                 uint32_t RefreshMode);
int32_t MicronSteer_RefreshRate(SDRAM_HandleTypeDef *Ctx,
                                 uint32_t RefreshCount);
int32_t MicronSteer_EnterPowerMode(SDRAM_HandleTypeDef *Ctx,
                                    uint32_t Interface);
int32_t MicronSteer_ExitPowerMode(SDRAM_HandleTypeDef *Ctx,
                                   uint32_t Interface);
int32_t MicronSteer_Sendcmd(SDRAM_HandleTypeDef *Ctx,
                             FMC_SDRAM_CommandTypeDef *SdramCmd);

/**
 * @}
 */

#ifdef __cplusplus
}
#endif

#endif /* MICRON_STEER_H */

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
