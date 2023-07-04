/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    fdcan.h
 * @brief   This file contains all the function prototypes for
 *          the fdcan.c file
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FDCAN_H__
#define __FDCAN_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */
#include "lvgl.h"
#include "primary/primary_network.h"
#include "primary/primary_watchdog.h"
#include "secondary/secondary_network.h"
/* USER CODE END Includes */

extern FDCAN_HandleTypeDef hfdcan1;

extern FDCAN_HandleTypeDef hfdcan2;

/* USER CODE BEGIN Private defines */

#define CHECK_SIZE(type)                                                       \
  if (msg->size != PRIMARY_##type##_BYTE_SIZE)                                 \
  print("Invalid size for" #type "message")

#define STEER_CAN_UNPACK(ntw, NTW, msg_name, MSG_NAME)                         \
  ntw##_##msg_name##_t raw;                                                    \
  ntw##_##msg_name##_converted_t converted;                                    \
  ntw##_##msg_name##_unpack(&raw, msg->data, PRIMARY_##MSG_NAME##_BYTE_SIZE);  \
  ntw##_##msg_name##_raw_to_conversion_struct(&converted, &raw);

#define STEER_CAN_PACK(ntw, NTW, msg_name, MSG_NAME)                           \
  can_message_t msg = {0};                                                     \
  msg.id = NTW##_##MSG_NAME##_FRAME_ID;                                        \
  msg.size = NTW##_##MSG_NAME##_BYTE_SIZE;                                     \
  ntw##_##msg_name##_t raw = {0};                                              \
  ntw##_##msg_name##_conversion_to_raw_struct(&raw, &converted);               \
  ntw##_##msg_name##_pack(msg.data, &raw, PRIMARY_##MSG_NAME##_BYTE_SIZE);

#define CAN_PRIMARY_NETWORK hfdcan1
#define CAN_SECONDARY_NETWORK hfdcan2
#define CAN_LOG_ENABLED 0

/* USER CODE END Private defines */

void MX_FDCAN1_Init(void);
void MX_FDCAN2_Init(void);

/* USER CODE BEGIN Prototypes */

typedef uint16_t can_id_t;

typedef struct {
  can_id_t id;
  uint8_t size;
  uint8_t data[8];
} can_message_t;

HAL_StatusTypeDef can_send(can_message_t *msg, FDCAN_HandleTypeDef *nwk);
void send_steer_version(lv_timer_t *);
void send_steer_status(lv_timer_t *);
void init_can_device(device_t *);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __FDCAN_H__ */
