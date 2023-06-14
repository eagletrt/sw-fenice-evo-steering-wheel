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

extern steering_t steering;

void _CAN_error_handler(char *msg);
void _CAN_Init_primary();
void _CAN_Init_secondary();

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
  hfdcan1.Init.NominalPrescaler = 2;
  hfdcan1.Init.NominalSyncJumpWidth = 16;
  hfdcan1.Init.NominalTimeSeg1 = 9;
  hfdcan1.Init.NominalTimeSeg2 = 2;
  hfdcan1.Init.DataPrescaler = 1;
  hfdcan1.Init.DataSyncJumpWidth = 1;
  hfdcan1.Init.DataTimeSeg1 = 2;
  hfdcan1.Init.DataTimeSeg2 = 1;
  hfdcan1.Init.MessageRAMOffset = 0;
  hfdcan1.Init.StdFiltersNbr = 1;
  hfdcan1.Init.ExtFiltersNbr = 0;
  hfdcan1.Init.RxFifo0ElmtsNbr = 10;
  hfdcan1.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.RxFifo1ElmtsNbr = 0;
  hfdcan1.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.RxBuffersNbr = 0;
  hfdcan1.Init.RxBufferSize = FDCAN_DATA_BYTES_8;
  hfdcan1.Init.TxEventsNbr = 0;
  hfdcan1.Init.TxBuffersNbr = 0;
  hfdcan1.Init.TxFifoQueueElmtsNbr = 10;
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
  hfdcan2.Init.NominalPrescaler = 16;
  hfdcan2.Init.NominalSyncJumpWidth = 16;
  hfdcan2.Init.NominalTimeSeg1 = 2;
  hfdcan2.Init.NominalTimeSeg2 = 2;
  hfdcan2.Init.DataPrescaler = 1;
  hfdcan2.Init.DataSyncJumpWidth = 1;
  hfdcan2.Init.DataTimeSeg1 = 2;
  hfdcan2.Init.DataTimeSeg2 = 1;
  hfdcan2.Init.MessageRAMOffset = 0;
  hfdcan2.Init.StdFiltersNbr = 1;
  hfdcan2.Init.ExtFiltersNbr = 0;
  hfdcan2.Init.RxFifo0ElmtsNbr = 10;
  hfdcan2.Init.RxFifo0ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan2.Init.RxFifo1ElmtsNbr = 0;
  hfdcan2.Init.RxFifo1ElmtSize = FDCAN_DATA_BYTES_8;
  hfdcan2.Init.RxBuffersNbr = 0;
  hfdcan2.Init.RxBufferSize = FDCAN_DATA_BYTES_8;
  hfdcan2.Init.TxEventsNbr = 0;
  hfdcan2.Init.TxBuffersNbr = 0;
  hfdcan2.Init.TxFifoQueueElmtsNbr = 10;
  hfdcan2.Init.TxFifoQueueMode = FDCAN_TX_FIFO_OPERATION;
  hfdcan2.Init.TxElmtSize = FDCAN_DATA_BYTES_8;
  if (HAL_FDCAN_Init(&hfdcan2) != HAL_OK) {
    Error_Handler();
  }
  /* USER CODE BEGIN FDCAN2_Init 2 */
  _CAN_Init_secondary();

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
    /* USER CODE BEGIN FDCAN2_MspDeInit 1 */

    /* USER CODE END FDCAN2_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

void _CAN_error_handler(char *msg) { printf("%s\n", msg); }

/**
 * @brief Create the CAN filter for the primary CAN network
 * @param f A CAN_FilterTypeDef in which to store the filter data
 * */
void _CAN_Init_primary() {
  FDCAN_FilterTypeDef f;
  HAL_StatusTypeDef s;
  f.IdType = FDCAN_STANDARD_ID;
  f.FilterIndex = 0;
  f.FilterType = FDCAN_FILTER_MASK;
  f.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  f.FilterID1 = 0;        // ???
  f.FilterID2 = 0;        // ???
  f.RxBufferIndex = 0;    // ignored
  f.IsCalibrationMsg = 0; // ignored

  /*
  Previous configuration:

  f->FilterMode = CAN_FILTERMODE_IDMASK;
  f->FilterIdLow = 0;
  f->FilterIdHigh = 0xFFFF;
  f->FilterMaskIdHigh = 0;
  f->FilterMaskIdLow = 0;
  f->FilterFIFOAssignment = CAN_FILTER_FIFO0;
  f->FilterBank = 0;
  f->FilterScale = CAN_FILTERSCALE_16BIT;
  f->FilterActivation = ENABLE;
  */

  if ((s = HAL_FDCAN_ConfigFilter(&hfdcan1, &f)) != HAL_OK)
    print("Failed to initialize CAN1 filter\n");

  if ((s = HAL_FDCAN_ActivateNotification(
           &hfdcan1, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0)) != HAL_OK)
    print("Failed to activate CAN1 interrupt\n");

  if ((s = HAL_FDCAN_Start(&hfdcan1)) != HAL_OK)
    print("Failed to start CAN1\n");
}

/**
 * @brief Create the CAN filter for the secondary CAN network
 * @param f A CAN_FilterTypeDef in which to store the filter data
 * */
void _CAN_Init_secondary() {
  FDCAN_FilterTypeDef f;
  HAL_StatusTypeDef s;
  f.IdType = FDCAN_STANDARD_ID;
  f.FilterIndex = 0;
  f.FilterType = FDCAN_FILTER_MASK;
  f.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  f.FilterID1 = 0;        // ???
  f.FilterID2 = 0;        // ???
  f.RxBufferIndex = 0;    // ignored
  f.IsCalibrationMsg = 0; // ignored

  /*
  Previous configuration:

  f->FilterMode = CAN_FILTERMODE_IDMASK;
  f->FilterIdLow = 0;
  f->FilterIdHigh = 0xFFFF;
  f->FilterMaskIdHigh = 0;
  f->FilterMaskIdLow = 0;
  f->FilterFIFOAssignment = CAN_FILTER_FIFO1;
  f->FilterBank = 1;
  f->FilterScale = CAN_FILTERSCALE_16BIT;
  f->FilterActivation = ENABLE;
  f->SlaveStartFilterBank = 1;
  */

  if ((s = HAL_FDCAN_ConfigFilter(&hfdcan2, &f)) != HAL_OK)
    print("Failed to initialize CAN2 filter\n");

  if ((s = HAL_FDCAN_ActivateNotification(
           &hfdcan2, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0)) != HAL_OK)
    print("Failed to activate CAN2 interrupt\n");

  if ((s = HAL_FDCAN_Start(&hfdcan2)) != HAL_OK)
    print("Failed to start CAN2\n");
}

void _can_wait(FDCAN_HandleTypeDef *nwk) {
  uint32_t start_timestamp = HAL_GetTick();
  while (HAL_FDCAN_GetTxFifoFreeLevel(nwk) == 0)
    if (HAL_GetTick() > start_timestamp + 5)
      return;
}

HAL_StatusTypeDef can_send(can_message_t *msg, FDCAN_HandleTypeDef *nwk) {

  FDCAN_TxHeaderTypeDef header = {
      .Identifier = msg->id,
      .IdType = FDCAN_STANDARD_ID,
      .TxFrameType = FDCAN_DATA_FRAME,
      .DataLength = msg->size,
      .ErrorStateIndicator = FDCAN_ESI_ACTIVE, // error active
      .BitRateSwitch = FDCAN_BRS_OFF,          // disable bit rate switching
      .FDFormat = FDCAN_CLASSIC_CAN,
      .TxEventFifoControl = FDCAN_STORE_TX_EVENTS,
      .MessageMarker = 0,
  };

  _can_wait(nwk);

  return HAL_FDCAN_AddMessageToTxFifoQ(nwk, &header, msg->data);
}

void handle_primary(can_message_t *msg) {
  switch (msg->id) {
  case PRIMARY_CAR_STATUS_FRAME_ID:
    CHECK_SIZE(CAR_STATUS);
    PRIMARY_UNPACK(car_status);
    // car_status_update(&data);
    break;
  default:
    break;
  }
}

void handle_secondary(can_message_t *msg) {}

/**
 * @brief     HAL callback for RX on FIFO0
 */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan,
                               uint32_t RxFifo0ITs) {
  print("RECEIVED MESSAGE CAN 0\n");
  FDCAN_RxHeaderTypeDef header;
  can_message_t msg;

  /* Get the last message */
  HAL_FDCAN_GetRxMessage(hfdcan, RxFifo0ITs, &header, msg.data);
  msg.id = header.IdType;
  msg.size = header.DataLength;

  handle_primary(&msg);
}

/**
 * @brief     HAL callback for RX on FIFO1
 */
void HAL_FDCAN_RxFifo1Callback(FDCAN_HandleTypeDef *hfdcan,
                               uint32_t RxFifo1ITs) {
  print("RECEIVED MESSAGE CAN 1\n");
  FDCAN_RxHeaderTypeDef header;
  can_message_t msg;

  /* Get the last message */
  HAL_FDCAN_GetRxMessage(hfdcan, RxFifo1ITs, &header, msg.data);
  msg.id = header.IdType;
  msg.size = header.DataLength;

  handle_secondary(&msg);
}

/* USER CODE END 1 */
