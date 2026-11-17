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

#include "can-communications.h"

#include <stdbool.h>

/* USER CODE END Includes */

extern FDCAN_HandleTypeDef hfdcan1;

extern FDCAN_HandleTypeDef hfdcan2;

/* USER CODE BEGIN Private defines */

/*!
 * \brief Transmit health of one FDCAN, to tell apart why frames don't go out.
 *
 * \details A wheel that receives but sends nothing shows one of three
 *     patterns: bus_offs and transmit_errors climbing (the frames leave the
 *     controller but never reach the bus: transceiver / TX line), unjams
 *     climbing with transmit_errors low (requests stuck in the Tx FIFO), or
 *     restricted_exits climbing (restricted operation mode).
 */
struct FdcanTxDiagnostics {
    uint32_t transmit_errors;  /*!< ECR.TEC, 0..255 (bus-off past 255) */
    uint32_t receive_errors;   /*!< ECR.REC, 0..127 */
    uint32_t last_error_code;  /*!< PSR.LEC: 0 none, 1 stuff, 2 form, 3 ack, 4 bit1, 5 bit0, 6 crc, 7 unchanged */
    bool error_passive;        /*!< PSR.EP */
    uint32_t tx_fifo_free;     /*!< TXFQS.TFFL, free Tx FIFO elements */
    uint32_t refused;          /*!< Frames refused by a full Tx FIFO since boot */
    uint32_t unjams;           /*!< Times every pending request was cancelled */
    uint32_t restricted_exits; /*!< Times restricted operation mode was left */
    uint32_t bus_offs;         /*!< Bus-off restarts since boot */
};

/* USER CODE END Private defines */

void MX_FDCAN1_Init(void);
void MX_FDCAN2_Init(void);

/* USER CODE BEGIN Prototypes */

/*!
 * \brief Start both FDCAN peripherals.
 *
 * \details Must be called once after MX_FDCAN1_Init and MX_FDCAN2_Init;
 *     filters and RX notifications are configured by the init functions.
 */
void fdcan_start(void);

/*!
 * \brief Send a CAN frame on the primary CAN network.
 *
 * \param frame The frame to send. Must not be NULL.
 *
 * \retval CAN_COMMUNICATION_RC_OK if the frame was sent successfully.
 * \retval CAN_COMMUNICATION_RC_NULL_POINTER if the frame pointer was NULL.
 * \retval CAN_COMMUNICATION_RC_INVALID_LENGTH if the frame length exceeds CAN_COMMUNICATIONS_FRAME_DATA_SIZE.
 * \retval CAN_COMMUNICATION_RC_TRANSMISSION_ERROR if the underlying HAL call reported a failure.
 */
enum CanCommunicationReturnCode fdcan_send_primary(const struct CanCommunicationFrame *frame);

/*!
 * \brief Send a CAN frame on the secondary CAN network.
 *
 * \param frame The frame to send. Must not be NULL.
 *
 * \retval CAN_COMMUNICATION_RC_OK if the frame was sent successfully.
 * \retval CAN_COMMUNICATION_RC_NULL_POINTER if the frame pointer was NULL.
 * \retval CAN_COMMUNICATION_RC_INVALID_LENGTH if the frame length exceeds CAN_COMMUNICATIONS_FRAME_DATA_SIZE.
 * \retval CAN_COMMUNICATION_RC_TRANSMISSION_ERROR if the underlying HAL call reported a failure.
 */
enum CanCommunicationReturnCode fdcan_send_secondary(const struct CanCommunicationFrame *frame);

/*!
 * \brief Number of times a network went bus-off (and was restarted) since boot.
 *
 * \param network CAN_COMMUNICATION_NETWORK_PRIMARY or CAN_COMMUNICATION_NETWORK_SECONDARY.
 *
 * \return The count.
 */
uint32_t fdcan_get_bus_off_count(enum CanCommunicationNetwork network);

/*!
 * \brief Whether a network's filter list had more entries than its StdFiltersNbr (.ioc).
 *
 * \details Then only the first entries were programmed; the POST fails.
 *
 * \return true if some identifiers the router decodes are dropped in hardware.
 */
bool fdcan_filters_overflow(void);

/*!
 * \brief Read the transmit health of a network.
 *
 * \param network CAN_COMMUNICATION_NETWORK_PRIMARY or CAN_COMMUNICATION_NETWORK_SECONDARY.
 * \param[out] diagnostics Filled with the current registers and counters (ignored if NULL).
 */
void fdcan_get_tx_diagnostics(enum CanCommunicationNetwork network, struct FdcanTxDiagnostics *diagnostics);

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif

#endif /* __FDCAN_H__ */
