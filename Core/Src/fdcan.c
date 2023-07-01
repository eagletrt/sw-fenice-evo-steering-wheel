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
device_t can_device;
uint8_t _raw[primary_MAX_STRUCT_SIZE_RAW];
uint8_t _converted[primary_MAX_STRUCT_SIZE_CONVERSION];
uint8_t payload[8];

void _CAN_error_handler(char *msg);
void _CAN_Init_primary();
void _CAN_Init_secondary();

/* USER CODE END 0 */

FDCAN_HandleTypeDef hfdcan1;
FDCAN_HandleTypeDef hfdcan2;

/* FDCAN1 init function */
void MX_FDCAN1_Init(void) {

  /* USER CODE BEGIN FDCAN1_Init 0 */

  init_can_device();

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
  hfdcan2.Init.NominalPrescaler = 2;
  hfdcan2.Init.NominalSyncJumpWidth = 64;
  hfdcan2.Init.NominalTimeSeg1 = 9;
  hfdcan2.Init.NominalTimeSeg2 = 2;
  hfdcan2.Init.DataPrescaler = 1;
  hfdcan2.Init.DataSyncJumpWidth = 1;
  hfdcan2.Init.DataTimeSeg1 = 2;
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

void _CAN_error_handler(char *msg) { printf("%s\n", msg); }

void init_can_device(void) {
  device_init(&can_device);
  device_set_address(&can_device, &_raw, primary_MAX_STRUCT_SIZE_RAW,
                     &_converted, primary_MAX_STRUCT_SIZE_CONVERSION);
}

/**
 * @brief Create the CAN filter for the primary CAN network
 * @param f A CAN_FilterTypeDef in which to store the filter data
 * */
void _CAN_Init_primary() {
  FDCAN_FilterTypeDef f;
  HAL_StatusTypeDef s;
  f.IdType = FDCAN_STANDARD_ID;
  f.FilterIndex = 0;
  f.FilterType = FDCAN_FILTER_RANGE;
  f.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  f.FilterID1 = 0;
  f.FilterID2 = 0x7FF;
  f.RxBufferIndex = 0;
  f.IsCalibrationMsg = 0;
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
  f.FilterConfig = FDCAN_FILTER_TO_RXFIFO1;
  f.FilterID1 = 0;
  f.FilterID2 = 0;
  f.RxBufferIndex = 0;
  f.IsCalibrationMsg = 0;

  if ((s = HAL_FDCAN_ConfigFilter(&hfdcan2, &f)) != HAL_OK)
    print("Failed to initialize CAN2 filter\n");

  if ((s = HAL_FDCAN_ActivateNotification(
           &hfdcan2, FDCAN_IT_RX_FIFO1_NEW_MESSAGE, 0)) != HAL_OK)
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

void send_steer_version(lv_timer_t *main_timer) {
  can_message_t msg = {0};
  msg.id = PRIMARY_STEER_VERSION_FRAME_ID;
  msg.size = PRIMARY_STEER_VERSION_BYTE_SIZE;
  primary_steer_version_t version = {.canlib_build_time = CANLIB_BUILD_TIME,
                                     .component_version = 1};
  primary_steer_version_pack(msg.data, &version,
                             PRIMARY_STEER_VERSION_BYTE_SIZE);
  can_send(&msg, &hfdcan1);
}

void send_steer_status(lv_timer_t *main_timer) {
  can_message_t msg = {0};
  msg.id = PRIMARY_STEER_STATUS_FRAME_ID;
  msg.size = PRIMARY_STEER_STATUS_BYTE_SIZE;
  primary_steer_status_t status = {.map_pw = steering.control.power,
                                   .map_sc = steering.control.slip,
                                   .map_tv = steering.control.torque};
  primary_steer_status_pack(msg.data, &status, PRIMARY_STEER_STATUS_BYTE_SIZE);
  can_send(&msg, &hfdcan1);
}

HAL_StatusTypeDef can_send(can_message_t *msg, FDCAN_HandleTypeDef *nwk) {

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
      .ErrorStateIndicator = FDCAN_ESI_ACTIVE, // error active
      .BitRateSwitch = FDCAN_BRS_OFF,          // disable bit rate switching
      .FDFormat = FDCAN_CLASSIC_CAN,
      .TxEventFifoControl = FDCAN_STORE_TX_EVENTS,
      .MessageMarker = 0,
  };

  // TODO CHECK if wait is necessary
  // _can_wait(nwk);

  return HAL_FDCAN_AddMessageToTxFifoQ(nwk, &header, msg->data);
}

#define CAST_FROM_DEVICE(message_name)                                         \
  primary_##message_name##_t *data =                                           \
      (primary_##message_name##_converted_t *)can_device.message

void handle_primary(can_message_t *msg) {
#if CAL_LOG_ENABLED
  char name_buffer[BUFSIZ];
  primary_message_name_from_id(msg->id, name_buffer);
  print("Primary network - message id %s\n", name_buffer);
#endif
  can_id_t id = msg->id;
  primary_devices_deserialize_from_id(&can_device, id, payload);
  switch (id) {
  case PRIMARY_CAR_STATUS_FRAME_ID: {
    CAST_FROM_DEVICE(car_status);
    car_status_update(data);
    break;
  }
  case PRIMARY_PEDAL_CALIBRATION_ACK_FRAME_ID: {
    CAST_FROM_DEVICE(pedal_calibration_ack);
    pedal_calibration_ack(data);
    break;
  }
  case PRIMARY_STEERING_JMP_TO_BLT_FRAME_ID:
    print("Resetting for open blt\n");
    HAL_NVIC_SystemReset();
    break;
  case PRIMARY_PTT_STATUS_FRAME_ID: {
    primary_ptt_status_t data;
    primary_ptt_status_unpack(&data, msg->data, msg->size);
    handle_ptt_message(data.status);
    break;
  }
  case PRIMARY_TLM_STATUS_FRAME_ID: {
    CAST_FROM_DEVICE(tlm_status);
    tlm_status_update(data);
    break;
  }
  case PRIMARY_HV_ERRORS_FRAME_ID: {
    CAST_FROM_DEVICE(hv_errors);
    hv_errors_update(data);
    break;
  }
  case PRIMARY_LV_ERRORS_FRAME_ID: {
    CAST_FROM_DEVICE(lv_errors);
    lv_errors_update(data);
    break;
  }
  case PRIMARY_HV_FEEDBACKS_STATUS_FRAME_ID: {
    CAST_FROM_DEVICE(hv_feedbacks_status);
    hv_feedback_update(data);
    break;
  }
  case PRIMARY_HV_TEMP_FRAME_ID: {
    CAST_FROM_DEVICE(hv_temp);
    hv_temp_update(data);
    break;
  }
  case PRIMARY_LV_TOTAL_VOLTAGE_FRAME_ID: {
    CAST_FROM_DEVICE(lv_total_voltage);
    lv_total_voltage_update(data);
    break;
  }
  case PRIMARY_LV_CELLS_VOLTAGE_FRAME_ID: {
    CAST_FROM_DEVICE(lv_cells_voltage);
    lv_cells_voltage_update(data);
    break;
  }
  case PRIMARY_LV_CURRENTS_FRAME_ID: {
    CAST_FROM_DEVICE(lv_currents);
    lv_currents_update(data);
    break;
  }
  case PRIMARY_LV_CELLS_TEMP_FRAME_ID: {
    CAST_FROM_DEVICE(lv_cells_temp);
    lv_cells_temp_update(data);
    break;
  }
  }
}

void handle_secondary(can_message_t *msg) {
#if CAN_LOG_ENABLED
  print("Secondary network - message id %" PRIu16 "\n", msg->id);
#endif
}

void HAL_FDCAN_ErrorCallback(FDCAN_HandleTypeDef *hfdcan) {
  if (hfdcan == &hfdcan1) {
    print("CAN0 error %" PRIu32 "\n", (uint32_t)hfdcan->ErrorCode);
  } else {
    print("CAN1 error %" PRIu32 "\n", (uint32_t)hfdcan->ErrorCode);
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
  msg.size = header.DataLength;
  if (hfdcan == &hfdcan1) {

#if CAN_LOG_ENABLED
    print("DEVICE 0\n");
#endif

    handle_primary(&msg);
  } else {
#if CAN_LOG_ENABLED
    print("DEVICE 1\n");
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
  msg.size = header.DataLength;
  if (hfdcan == &hfdcan1) {

#if CAN_LOG_ENABLED
    print("DEVICE 0\n");
#endif

    handle_primary(&msg);
  } else {

#if CAN_LOG_ENABLED
    print("DEVICE 1\n");
#endif

    handle_secondary(&msg);
  }
}

/* USER CODE END 1 */
