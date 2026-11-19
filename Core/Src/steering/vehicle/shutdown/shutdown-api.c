/*!
 * \file shutdown-api.c
 * \date 2026-11-19
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Implementation of the shutdown chain evaluation.
 */

#include "vehicle/shutdown/shutdown-api.h"

#include <stddef.h>

bool shutdown_api_evaluate(const float *voltage, uint32_t available, struct ShutdownChain *chain) {
    if (voltage == NULL || chain == NULL) {
        return false;
    }
    chain->cause = SHUTDOWN_NODE_COUNT;
    for (uint8_t node = 0U; node < SHUTDOWN_NODE_COUNT; node++) {
        if ((available & SHUTDOWN_NODE_BIT(node)) == 0U) {
            chain->state[node] = SHUTDOWN_STATE_UNAVAILABLE;
        } else if (chain->cause != SHUTDOWN_NODE_COUNT) {
            // Past the cause nothing can be closed: whatever it reads, it has no supply.
            chain->state[node] = SHUTDOWN_STATE_DOWNSTREAM;
        } else if (voltage[node] >= SHUTDOWN_CLOSED_V) {
            chain->state[node] = SHUTDOWN_STATE_CLOSED;
        } else {
            chain->state[node] = SHUTDOWN_STATE_OPEN;
            chain->cause = (enum ShutdownNode)node;
        }
    }
    return true;
}
