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
    SHUTDOWN_NODE_AMS,
    SHUTDOWN_NODE_IMD,
    SHUTDOWN_NODE_TSMS,
    SHUTDOWN_NODE_BOTS,
    SHUTDOWN_NODE_INERTIA,
    SHUTDOWN_NODE_BSPD,
    SHUTDOWN_NODE_COCKPIT_MUSHROOM,
    SHUTDOWN_NODE_LEFT_MUSHROOM,
    SHUTDOWN_NODE_RIGHT_MUSHROOM,
    SHUTDOWN_NODE_HVD,
    SHUTDOWN_NODE_TSAC_INTERLOCK,
    SHUTDOWN_NODE_INVERTER_INTERLOCK, /*!< TODO(libcan) */
    SHUTDOWN_NODE_ASMS,
    SHUTDOWN_NODE_RES,
    SHUTDOWN_NODE_EBS,
    SHUTDOWN_NODE_SDC_RELAY, /*!< TODO(libcan) */
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
