/*!
 * \file shutdown-api.c
 * \date 2026-11-19
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Implementation of the shutdown chain evaluation.
 */

#include "vehicle/shutdown/shutdown-api.h"

#include "eagletrt.h"

#include <stddef.h>

EAGLETRT_STATIC const char *const shutdown_node_names[SHUTDOWN_NODE_COUNT] = {
    [SHUTDOWN_NODE_ECU] = "ECU",
    [SHUTDOWN_NODE_COCKPIT_MUSHROOM] = "COCKPIT MUSHROOM",
    [SHUTDOWN_NODE_FRONT_RIGHT_A_ARM] = "FRONT RIGHT A-ARM",
    [SHUTDOWN_NODE_BOTS] = "BOTS",
    [SHUTDOWN_NODE_FRONT_LEFT_A_ARM] = "FRONT LEFT A-ARM",
    [SHUTDOWN_NODE_INERTIA] = "INERTIA",
    [SHUTDOWN_NODE_INVERTER_INTERLOCK] = "INVERTER",
    [SHUTDOWN_NODE_REAR_LEFT_A_ARM] = "REAR LEFT A-ARM",
    [SHUTDOWN_NODE_BSPD] = "BSPD",
    [SHUTDOWN_NODE_HVD] = "HVD",
    [SHUTDOWN_NODE_LEFT_MUSHROOM] = "LEFT MUSHROOM",
    [SHUTDOWN_NODE_RIGHT_MUSHROOM] = "RIGHT MUSHROOM",
    [SHUTDOWN_NODE_AMS] = "AMS",
    [SHUTDOWN_NODE_IMD] = "IMD",
    [SHUTDOWN_NODE_REAR_RIGHT_A_ARM] = "REAR RIGHT A-ARM",
    [SHUTDOWN_NODE_TSMS] = "TSMS",
    [SHUTDOWN_NODE_SD_END] = "SD END",
};

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

const char *shutdown_api_node_name(enum ShutdownNode node) {
    return ((uint8_t)node < SHUTDOWN_NODE_COUNT) ? shutdown_node_names[node] : "";
}
