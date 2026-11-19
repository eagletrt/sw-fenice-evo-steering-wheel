/*!
 * \file shutdown.h
 * \date 2026-11-19
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Definitions of the shutdown circuit: its nodes in circuit order and
 *     what each one is doing.
 */

#ifndef SHUTDOWN_H
#define SHUTDOWN_H

#include <stdint.h>

#define SHUTDOWN_CLOSED_V (12.0F) /*!< TODO(design): a node at or above this voltage is closed */

/*!
 * \brief Shutdown circuit nodes, in circuit order (the first open one is the cause).
 */
enum ShutdownNode : uint8_t {
    SHUTDOWN_NODE_ECU,                /*!< After the ECU (ShutdownState.voltageOut) */
    SHUTDOWN_NODE_COCKPIT_MUSHROOM,   /*!< DasShutdown */
    SHUTDOWN_NODE_FRONT_RIGHT_A_ARM,  /*!< TODO(libcan): not read */
    SHUTDOWN_NODE_BOTS,               /*!< PedalsBrake */
    SHUTDOWN_NODE_FRONT_LEFT_A_ARM,   /*!< TODO(libcan): not read */
    SHUTDOWN_NODE_INERTIA,            /*!< DasShutdown */
    SHUTDOWN_NODE_INVERTER_INTERLOCK, /*!< TODO(libcan): the five interlocks, not read */
    SHUTDOWN_NODE_REAR_LEFT_A_ARM,    /*!< TODO(libcan): not read */
    SHUTDOWN_NODE_BSPD,               /*!< PipoShutdown */
    SHUTDOWN_NODE_HVD,                /*!< PipoShutdown */
    SHUTDOWN_NODE_LEFT_MUSHROOM,      /*!< DasShutdown */
    SHUTDOWN_NODE_RIGHT_MUSHROOM,     /*!< DasShutdown */
    SHUTDOWN_NODE_AMS,                /*!< TsacMainboardShutdown */
    SHUTDOWN_NODE_IMD,                /*!< TsacMainboardShutdown */
    SHUTDOWN_NODE_REAR_RIGHT_A_ARM,   /*!< TODO(libcan): not read */
    SHUTDOWN_NODE_TSMS,               /*!< PipoShutdown */
    SHUTDOWN_NODE_SD_END,             /*!< sdEnd: AIRs and inverter enable (TsacMainboardFeedbackShutdown) */
    SHUTDOWN_NODE_COUNT,
};

/*!
 * \brief What a node is doing.
 */
enum ShutdownNodeState : uint8_t {
    SHUTDOWN_STATE_CLOSED,      /*!< Voltage after the node at or above SHUTDOWN_CLOSED_V */
    SHUTDOWN_STATE_OPEN,        /*!< The first open node: the cause */
    SHUTDOWN_STATE_DOWNSTREAM,  /*!< Open because a node before it is */
    SHUTDOWN_STATE_UNAVAILABLE, /*!< No reading (stale or not on CAN): skipped, the chain goes on */
};

/*!
 * \brief The whole chain, evaluated.
 */
struct ShutdownChain {
    enum ShutdownNodeState state[SHUTDOWN_NODE_COUNT];
    enum ShutdownNode cause; /*!< The first open node, SHUTDOWN_NODE_COUNT when every available node is closed */
};

#endif // SHUTDOWN_H
