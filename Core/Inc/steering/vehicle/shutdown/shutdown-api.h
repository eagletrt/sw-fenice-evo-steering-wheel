/*!
 * \file shutdown-api.h
 * \date 2026-11-19
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Public API of the shutdown circuit: walk the chain in order.
 */

#ifndef SHUTDOWN_API_H
#define SHUTDOWN_API_H

#include "vehicle/shutdown/shutdown.h"

#include <stdbool.h>

/*!
 * \brief Bit of a node in the \p available mask of shutdown_api_evaluate.
 */
#define SHUTDOWN_NODE_BIT(node) ((uint32_t)1U << (uint8_t)(node))

/*!
 * \brief Walk the chain in circuit order.
 *
 * \details Each available node is CLOSED at or above SHUTDOWN_CLOSED_V. The
 *     first available node below it is OPEN (the cause), every available
 *     node after it is DOWNSTREAM. A node without a reading is UNAVAILABLE and
 *     skipped: the nodes around it are judged as if it were not there, so a
 *     hole in the data never makes the rest of the chain look open.
 *
 * \param[in] voltage Voltage after each node, SHUTDOWN_NODE_COUNT entries.
 * \param available SHUTDOWN_NODE_BIT of every node with a fresh reading.
 * \param[out] chain The result.
 *
 * \return false if an argument is NULL (\p chain untouched), true otherwise.
 */
bool shutdown_api_evaluate(const float *voltage, uint32_t available, struct ShutdownChain *chain);

/*!
 * \brief Name of a node as the screens show it ("BSPD", "REAR LEFT A-ARM").
 *
 * \param node The node.
 * \return Its name, "" for SHUTDOWN_NODE_COUNT or an invalid node.
 */
const char *shutdown_api_node_name(enum ShutdownNode node);

#endif // SHUTDOWN_API_H
