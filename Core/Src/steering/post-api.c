/*!
 * \file post-api.c
 * \date 2026-07-13
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief This file defines Power-On Self-Test (POST) functions for system diagnostics.
 */

#include "post-api.h"
#include "input-events-api.h"
#include "inputs-api.h"
#include "parameters-api.h"
#include "can-communications-api.h"
#include "identity-api.h"
#include "screen-api.h"

enum PostReturnCode post_api_do_init(struct PostInitData *post_init_data) {
    if (post_init_data == NULL ||
        post_init_data->parameters_on_change == NULL ||
        post_init_data->draw_rectangle == NULL ||
        post_init_data->can_network_configs[0].on_receive == NULL ||
        post_init_data->can_network_configs[0].send == NULL ||
        post_init_data->can_network_configs[1].on_receive == NULL ||
        post_init_data->can_network_configs[1].send == NULL) {
        return POST_RC_ERROR;
    }

    enum PostReturnCode ret_code = POST_RC_OK;

    if (parameters_api_init(post_init_data->parameters_on_change) != PARAMETERS_RC_OK) {
        ret_code = POST_RC_ERROR;
    }

    // Every input goes through the event layer (combos), then the router
    // decides per FSM state what reaches the parameters.
    if (inputs_api_init(
            input_events_api_on_button_press,
            input_events_api_on_button_release,
            input_events_api_on_knob_rotation) != INPUTS_RC_OK) {
        ret_code = POST_RC_ERROR;
    }

    if (can_communications_api_init(post_init_data->can_network_configs) != CAN_COMMUNICATION_RC_OK) {
        ret_code = POST_RC_ERROR;
    }

    if (identity_api_init() != IDENTITY_RC_OK) {
        ret_code = POST_RC_ERROR;
    }

    if (screen_api_init(post_init_data->draw_rectangle) != SCREEN_RC_OK ||
        screen_api_set_accelerators(&post_init_data->screen_accelerators) != SCREEN_RC_OK) {
        ret_code = POST_RC_ERROR;
    }

    return ret_code;
}
