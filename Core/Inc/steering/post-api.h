/*!
 * \file post-api.h
 * \date 2026-07-13
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief This file defines Power-On Self-Test (POST) functions for system diagnostics.
 */

#ifndef POST_API_H
#define POST_API_H

#include "post.h"

/*!
 * \brief Performs the Power-On Self-Test (POST) using the provided initialization data.
 *
 * This function initializes all the modules required by the steering wheel firmware.
 *
 * \param post_init_data Pointer to a structure containing the necessary initialization data for POST.
 *
 * \retval POST_RC_OK if POST completed successfully
 * \retval POST_RC_ERROR if POST encountered an error
 */
enum PostReturnCode post_api_do_init(struct PostInitData *post_init_data);

#endif // POST_API_H
