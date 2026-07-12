/*!
 * \file ui-data-api.c
 * \date 2026-07-13
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Implementation of the UIData accessor.
 */

#include "ui-data-api.h"
#include "eagletrt.h"

EAGLETRT_STATIC struct UIData ui_data;

struct UIData *ui_data_api_get(void) {
    return &ui_data;
}
