/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    fdcan.c
 * @brief   This file provides code for the configuration
 *          of the FDCAN instances.
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
#include "fdcan.h"

/* USER CODE BEGIN 0 */

extern bool steering_initialized;

device_t primary_can_device;
device_t secondary_can_device;
uint8_t _raw[primary_MAX_STRUCT_SIZE_RAW];
uint8_t _converted[primary_MAX_STRUCT_SIZE_CONVERSION];

#if CANSNIFFER_ENABLED == 1
extern cansniffer_elem_t *primary_cansniffer_buffer;
extern cansniffer_elem_t *secondary_cansniffer_buffer;
#endif

void _CAN_error_handler(char *msg);
void _CAN_Init_primary();
void _CAN_Init_secondary();

#if DEBUG_RX_BUFFERS_ENABLED == 1
void debug_rx_count(bool is_primary, bool is_rx0);
#endif

bool primary_can_fatal_error = false;
bool secondary_can_fatal_error = false;

/* USER CODE END 0 */

FDCAN_HandleTypeDef hfdcan1;
FDCAN_HandleTypeDef hfdcan2;

/* FDCAN1 init function */
void MX_FDCAN1_Init(void) {

  /* USER CODE BEGIN FDCAN1_Init 0 */

  /* USER CODE END FDCAN1_Init 0 */

  /* USER CODE BEGIN FDCAN1_Init 1 */

  /* USER CODE END FDCAN1_Init 1 */
  hfdcan1.Instance = FDCAN1;
  hfdcan1.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan1.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan1.Init.AutoRetransmission = DISABLE;
  hfdcan1.Init.TransmitPause = DISABLE;
  hfdcan1.Init.ProtocolException = DISABLE;
  hfdcan1.Init.NominalPrescaler = 1;
  hfdcan1.Init.NominalSyncJumpWidth = 64;
  hfdcan1.Init.NominalTimeSeg1 = 18;
  hfdcan1.Init.NominalTimeSeg2 = 5;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 1;
  hfdcan1.Init.DataTimeSeg2 = 1;
  hfdcan1.Init.MessageRAMOffset = 0;
  hfdcan1.Init.StdFiltersNbr = 1;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.RxFifo0ElmtsNbr = 64;
  hfdcan1.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.RxFifo1ElmtsNbr = 64;
  hfdcan1.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.RxBuffersNbr = 0;
  hfdcan1.Init.RxBufferSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.TxEventsNbr = 32;
  hfdcan1.Init.TxBuffersNbr = 32;
  hfdcan1.Init.TxFifoQueueElmtsNbr = 32;
  hfdcan1.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  hfdcan1.Init.TxElmtSize = FDCAN_DATA_BYTES_8;
  if (HAL_FDCAN_Init(&hfdcan1) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN1_Init 2 */
  _CAN_Init_primary();

  /* USER CODE END FDCAN1_Init 2 */
}
/* FDCAN2 init function */
void MX_FDCAN2_Init(void) {

  /* USER CODE BEGIN FDCAN2_Init 0 */

  /* USER CODE END FDCAN2_Init 0 */

  /* USER CODE BEGIN FDCAN2_Init 1 */

  /* USER CODE END FDCAN2_Init 1 */
  hfdcan2.Instance = FDCAN2;
  hfdcan2.Init.FrameFormat = FDCAN_FRAME_CLASSIC;
  hfdcan2.Init.Mode = FDCAN_MODE_NORMAL;
  hfdcan2.Init.AutoRetransmission = DISABLE;
  hfdcan2.Init.TransmitPause = DISABLE;
  hfdcan2.Init.ProtocolException = DISABLE;
  hfdcan2.Init.NominalPrescaler = 1;
  hfdcan2.Init.NominalSyncJumpWidth = 64;
  hfdcan2.Init.NominalTimeSeg1 = 18;
  hfdcan2.Init.NominalTimeSeg2 = 5;
  hfdcan2.Init.DataPrescaler = 1;
  hfdcan2.Init.DataSyncJumpWidth = 1;
  hfdcan2.Init.DataTimeSeg1 = 1;
  hfdcan2.Init.DataTimeSeg2 = 1;
  hfdcan2.Init.MessageRAMOffset = 0;
  hfdcan2.Init.StdFiltersNbr = 1;
  hfdcan2.Init.ExtFiltersNbr = 0;
  hfdcan2.Init.RxFifo0ElmtsNbr = 64;
  hfdcan2.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan2.Init.RxFifo1ElmtsNbr = 64;
  hfdcan2.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan2.Init.RxBuffersNbr = 0;
  hfdcan2.Init.RxBufferSize = FDCAN_DATA_BYTES_8;
  hfdcan2.Init.TxEventsNbr = 32;
  hfdcan2.Init.TxBuffersNbr = 32;
  hfdcan2.Init.TxFifoQueueElmtsNbr = 32;
  hfdcan2.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  hfdcan2.Init.TxElmtSize = FDCAN_DATA_BYTES_8;
  if (HAL_FDCAN_Init(&hfdcan2) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN2_Init 2 */
  // _CAN_Init_secondary();

  /* USER CODE END FDCAN2_Init 2 */
}

static uint32_t HAL_RCC_FDCAN_CLK_ENABLED = 0;

void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef *fdcanHandle) {

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};
  if (fdcanHandle->Instance == FDCAN1) {
    /* USER CODE BEGIN FDCAN1_MspInit 0 */

    /* USER CODE END FDCAN1_MspInit 0 */

    /** Initializes the peripherals clock
     */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_HSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
      Error_Handler();
    }

    /* FDCAN1 clock enable */
    HAL_RCC_FDCAN_CLK_ENABLED++;
    if (HAL_RCC_FDCAN_CLK_ENABLED == 1) {
      __HAL_RCC_FDCAN_CLK_ENABLE();
    }

    __HAL_RCC_GPIOA_CLK_ENABLE();
    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    /* FDCAN1 interrupt Init */
    HAL_NVIC_SetPriority(FDCAN1_IT0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(FDCAN1_IT0_IRQn);
    HAL_NVIC_SetPriority(FDCAN1_IT1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(FDCAN1_IT1_IRQn);
    /* USER CODE BEGIN FDCAN1_MspInit 1 */

    /* USER CODE END FDCAN1_MspInit 1 */
  } else if (fdcanHandle->Instance == FDCAN2) {
    /* USER CODE BEGIN FDCAN2_MspInit 0 */

    /* USER CODE END FDCAN2_MspInit 0 */

    /** Initializes the peripherals clock
     */
    PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_FDCAN;
    PeriphClkInitStruct.FdcanClockSelection = RCC_FDCANCLKSOURCE_HSE;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK) {
      Error_Handler();
    }

    /* FDCAN2 clock enable */
    HAL_RCC_FDCAN_CLK_ENABLED++;
    if (HAL_RCC_FDCAN_CLK_ENABLED == 1) {
      __HAL_RCC_FDCAN_CLK_ENABLE();
    }

    __HAL_RCC_GPIOB_CLK_ENABLE();
    /**FDCAN2 GPIO Configuration
    PB12     ------> FDCAN2_RX
    PB13     ------> FDCAN2_TX
    */
    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_13;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF9_FDCAN2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* FDCAN2 interrupt Init */
    HAL_NVIC_SetPriority(FDCAN2_IT0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(FDCAN2_IT0_IRQn);
    HAL_NVIC_SetPriority(FDCAN2_IT1_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(FDCAN2_IT1_IRQn);
    /* USER CODE BEGIN FDCAN2_MspInit 1 */

    /* USER CODE END FDCAN2_MspInit 1 */
  }
}

void HAL_FDCAN_MspDeInit(FDCAN_HandleTypeDef *fdcanHandle) {

  if (fdcanHandle->Instance == FDCAN1) {
    /* USER CODE BEGIN FDCAN1_MspDeInit 0 */

    /* USER CODE END FDCAN1_MspDeInit 0 */
    /* Peripheral clock disable */
    HAL_RCC_FDCAN_CLK_ENABLED--;
    if (HAL_RCC_FDCAN_CLK_ENABLED == 0) {
      __HAL_RCC_FDCAN_CLK_DISABLE();
    }

    /**FDCAN1 GPIO Configuration
    PA11     ------> FDCAN1_RX
    PA12     ------> FDCAN1_TX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_11 | GPIO_PIN_12);

    /* FDCAN1 interrupt Deinit */
    HAL_NVIC_DisableIRQ(FDCAN1_IT0_IRQn);
    HAL_NVIC_DisableIRQ(FDCAN1_IT1_IRQn);
    /* USER CODE BEGIN FDCAN1_MspDeInit 1 */

    /* USER CODE END FDCAN1_MspDeInit 1 */
  } else if (fdcanHandle->Instance == FDCAN2) {
    /* USER CODE BEGIN FDCAN2_MspDeInit 0 */

    /* USER CODE END FDCAN2_MspDeInit 0 */
    /* Peripheral clock disable */
    HAL_RCC_FDCAN_CLK_ENABLED--;
    if (HAL_RCC_FDCAN_CLK_ENABLED == 0) {
      __HAL_RCC_FDCAN_CLK_DISABLE();
    }

    /**FDCAN2 GPIO Configuration
    PB12     ------> FDCAN2_RX
    PB13     ------> FDCAN2_TX
    */
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12 | GPIO_PIN_13);

    /* FDCAN2 interrupt Deinit */
    HAL_NVIC_DisableIRQ(FDCAN2_IT0_IRQn);
    HAL_NVIC_DisableIRQ(FDCAN2_IT1_IRQn);
    /* USER CODE BEGIN FDCAN2_MspDeInit 1 */

    /* USER CODE END FDCAN2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/**
 * @brief Create the CAN filter for the primary CAN network
 * @param f A CAN_FilterTypeDef in which to store the filter data
 * */
void _CAN_Init_primary() {
  FDCAN_FilterTypeDef f = {0};
  HAL_StatusTypeDef s = {0};
  f.IdType = FDCAN_STANDARD_ID;
  f.FilterIndex = 0;
  f.FilterType = FDCAN_FILTER_MASK;
  f.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  f.FilterID1 = ((1U << 11) - 1) << 5;
  f.FilterID2 = 0;
  f.IsCalibrationMsg = 0;
  if ((s = HAL_FDCAN_ConfigFilter(&hfdcan1, &f)) != HAL_OK) {
    primary_can_fatal_error = true;
  }

  if ((s = HAL_FDCAN_ActivateNotification(
           &hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0)) != HAL_OK) {
    primary_can_fatal_error = true;
  }

  if ((s = HAL_FDCAN_ConfigInterruptLines(
           &hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0)) != HAL_OK) {
    primary_can_fatal_error = true;
  }

  if ((s = HAL_FDCAN_Start(&hfdcan1)) != HAL_OK) {
    primary_can_fatal_error = true;
  }
}

/**
 * @brief Create the CAN filter for the secondary CAN network
 * @param f A CAN_FilterTypeDef in which to store the filter data
 * */
void _CAN_Init_secondary() {
  FDCAN_FilterTypeDef f = {0};
  HAL_StatusTypeDef s = {0};
  f.IdType = FDCAN_STANDARD_ID;
  f.FilterIndex = 0;
  f.FilterType = FDCAN_FILTER_MASK;
  f.FilterConfig = FDCAN_FILTER_TO_RXFIFO1;
  f.FilterID1 = ((1U << 11) - 1) << 5;
  f.FilterID2 = 0;
  f.IsCalibrationMsg = 0;
  if ((s = HAL_FDCAN_ConfigFilter(&hfdcan2, &f)) != HAL_OK) {
    secondary_can_fatal_error = true;
  }

  if ((s = HAL_FDCAN_ActivateNotification(
           &hfdcan2, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0)) != HAL_OK) {
    secondary_can_fatal_error = true;
  }

  if ((s = HAL_FDCAN_ConfigInterruptLines(
           &hfdcan2, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 1)) != HAL_OK) {
    secondary_can_fatal_error = true;
  }

  if ((s = HAL_FDCAN_Start(&hfdcan2)) != HAL_OK) {
    secondary_can_fatal_error = true;
  }
}

void _can_wait(FDCAN_HandleTypeDef *nwk) {
  uint32_t start_timestamp = HAL_GetTick();
  while (HAL_FDCAN_GetTxFifoFreeLevel(nwk) == 0)
    if (HAL_GetTick() > start_timestamp + 5)
      return;
}

bool can_send(can_message_t *msg, bool to_primary_network) {

  FDCAN_HandleTypeDef *nwk = to_primary_network ? &hfdcan1 : &hfdcan2;

  uint32_t dlc_len = 0;
  switch (msg->size) {
  case 0:
    dlc_len = FDCAN_DLC_BYTES_0;
    break;
  case 1:
    dlc_len = FDCAN_DLC_BYTES_1;
    break;
  case 2:
    dlc_len = FDCAN_DLC_BYTES_2;
    break;
  case 3:
    dlc_len = FDCAN_DLC_BYTES_3;
    break;
  case 4:
    dlc_len = FDCAN_DLC_BYTES_4;
    break;
  case 5:
    dlc_len = FDCAN_DLC_BYTES_5;
    break;
  case 6:
    dlc_len = FDCAN_DLC_BYTES_6;
    break;
  case 7:
    dlc_len = FDCAN_DLC_BYTES_7;
    break;
  case 8:
    dlc_len = FDCAN_DLC_BYTES_8;
    break;
  }

  FDCAN_TxHeaderTypeDef header = {
      .Identifier = msg->id,
      .IdType = FDCAN_STANDARD_ID,
      .TxFrameType = FDCAN_DATA_FRAME,
      .DataLength = dlc_len,
      .ErrorStateIndicator = FDCAN_ESI_ACTIVE,
      .BitRateSwitch = FDCAN_BRS_OFF,
      .FDFormat = FDCAN_CLASSIC_CAN,
      .TxEventFifoControl = FDCAN_STORE_TX_EVENTS,
      .MessageMarker = 0,
  };

  _can_wait(nwk);

  HAL_StatusTypeDef result =
      HAL_FDCAN_AddMessageToTxFifoQ(nwk, &header, msg->data);
  return result == HAL_OK;
}

void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan) {
  if (hfdcan == &hfdcan1) {
#ifdef STEERING_LOG_ENABLED
    print("CAN0 error %" PRIu32 "\n", (uint32_t)hfdcan->ErrorCode);
#endif
  } else {
#ifdef STEERING_LOG_ENABLED
    print("CAN1 error %" PRIu32 "\n", (uint32_t)hfdcan->ErrorCode);
#endif
  }
}

/**
 * @brief     HAL callback for RX on FIFO0
 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan,
                               uint32_t RxFifo0ITs) {
  FDCAN_RxHeaderTypeDef header = {0};
  can_message_t msg;
  HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &header, msg.data);

#if CAN_LOG_ENABLED
  uint32_t fill_level = HAL_FDCAN_GetRxFifoFillLevel(hfdcan, FDCAN_RX_FIFO0);
  print("RX Fifo0 fill level %" PRIu32 "\n", fill_level);
#endif

  msg.id = header.Identifier;
  msg.size = 0;

  switch (header.DataLength) {
  case FDCAN_DLC_BYTES_0:
    msg.size = 0;
    break;
  case FDCAN_DLC_BYTES_1:
    msg.size = 1;
    break;
  case FDCAN_DLC_BYTES_2:
    msg.size = 2;
    break;
  case FDCAN_DLC_BYTES_3:
    msg.size = 3;
    break;
  case FDCAN_DLC_BYTES_4:
    msg.size = 4;
    break;
  case FDCAN_DLC_BYTES_5:
    msg.size = 5;
    break;
  case FDCAN_DLC_BYTES_6:
    msg.size = 6;
    break;
  case FDCAN_DLC_BYTES_7:
    msg.size = 7;
    break;
  case FDCAN_DLC_BYTES_8:
    msg.size = 8;
    break;
  }

  if (hfdcan == &hfdcan1) {
#if DEBUG_RX_BUFFERS_ENABLED == 1
    debug_rx_count(true, true);
#endif
    handle_primary(&msg);
  } else if (hfdcan == &hfdcan2) {
#if DEBUG_RX_BUFFERS_ENABLED == 1
    debug_rx_count(false, true);
#endif
    handle_secondary(&msg);
  }
}

/**
 * @brief     HAL callback for RX on FIFO1
 */
void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan,
                               uint32_t RxFifo1ITs) {
  FDCAN_RxHeaderTypeDef header;
  can_message_t msg;
  HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO1, &header, msg.data);

#if CAN_LOG_ENABLED
  uint32_t fill_level = HAL_FDCAN_GetRxFifoFillLevel(hfdcan, FDCAN_RX_FIFO1);
  print("RX Fifo1 fill level %" PRIu32 "\n", fill_level);
#endif

  msg.id = header.Identifier;
  msg.size = 0;

  switch (header.DataLength) {
  case FDCAN_DLC_BYTES_0:
    msg.size = 0;
    break;
  case FDCAN_DLC_BYTES_1:
    msg.size = 1;
    break;
  case FDCAN_DLC_BYTES_2:
    msg.size = 2;
    break;
  case FDCAN_DLC_BYTES_3:
    msg.size = 3;
    break;
  case FDCAN_DLC_BYTES_4:
    msg.size = 4;
    break;
  case FDCAN_DLC_BYTES_5:
    msg.size = 5;
    break;
  case FDCAN_DLC_BYTES_6:
    msg.size = 6;
    break;
  case FDCAN_DLC_BYTES_7:
    msg.size = 7;
    break;
  case FDCAN_DLC_BYTES_8:
    msg.size = 8;
    break;
  }

  if (hfdcan == &hfdcan1) {
#if DEBUG_RX_BUFFERS_ENABLED == 1
    debug_rx_count(true, false);
#endif
    handle_primary(&msg);
  } else if (hfdcan == &hfdcan2) {
#if DEBUG_RX_BUFFERS_ENABLED == 1
    debug_rx_count(false, false);
#endif
    handle_secondary(&msg);
  }
}

#if DEBUG_RX_BUFFERS_ENABLED == 1
uint32_t debug_rx_counters[4] = {0};
void debug_rx_count(bool is_primary, bool is_rx0) {
  // { primary_rx0, primary_rx1, secondary_rx0, secondary_rx1 }
  debug_rx_counters[is_primary * 2 + is_rx0] += 1;
}
#endif

/* USER CODE END 1 */
