/*!
 * \file post.h
 * \date 2026-07-13
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief This file defines Power-On Self-Test (POST) structures for system diagnostics.
 */

#ifndef POST_H
#define POST_H

#include "leds.h"
#include "can-communications.h"
#include "parameters.h"
#include "raster.h"

enum PostReturnCode {
    POST_RC_OK,    /*!< POST completed successfully. */
    POST_RC_ERROR, /*!< POST encountered an error. */
};

struct PostInitData {
    leds_transmit_callback leds_transmit;                                                       /*!< Callback function required by leds module. */
    parameters_on_change_callback parameters_on_change;                                         /*!< Callback fired on every parameter transition. */
    struct CanCommunicationsNetworkConfig can_network_configs[CAN_COMMUNICATION_NETWORK_COUNT]; /*!< Configuration for each CAN network. */
    raster_draw_rectangle_callback draw_rectangle;                                              /*!< Callback function required by screen module. */
};

#endif // POST_H
