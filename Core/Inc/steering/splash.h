/*!
 * \file splash.h
 * \date 2026-10-29
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Definitions for the splash screen: team logo and an optional
 *     two-line caption.
 *
 * \details Shared by every full-screen "logo" page so they line up exactly:
 *     the FLASH page of the firmware, the bootloader screen and (later) the
 *     startup logo. The logo never moves; the caption below it is empty when
 *     there is nothing to say. Depends only on libraster, so the bootloader
 *     builds it too.
 */

#ifndef SPLASH_H
#define SPLASH_H

#include "box.h"
#include "label.h"
#include "splash-logo.h"
#include "style.h"

#include <stdint.h>

#define SPLASH_SCREEN_WIDTH (800U)  /*!< Width of the display, in pixels */
#define SPLASH_SCREEN_HEIGHT (480U) /*!< Height of the display, in pixels */

#define SPLASH_LOGO_X ((SPLASH_SCREEN_WIDTH - SPLASH_LOGO_WIDTH) / 2U) /*!< Left edge of the logo */
#define SPLASH_LOGO_Y (112U)                                           /*!< Top edge of the logo */

#define SPLASH_LOGO_BOX_HEIGHT (320U) /*!< Logo box: top of the screen down to the caption */
#define SPLASH_TITLE_BOX_HEIGHT (48U) /*!< Title box, right under the logo box */
#define SPLASH_TITLE_FONT_SIZE (30U)  /*!< Pixel height of the title text */
#define SPLASH_TITLE_OFFSET_Y (6U)    /*!< Title text top, from the top of its box */
#define SPLASH_NAME_FONT_SIZE (56U)   /*!< Pixel height of the name text */
#define SPLASH_NAME_OFFSET_Y (4U)     /*!< Name text top, from the top of its box */

#define SPLASH_COLOR_BACKGROUND (STYLE_BLACK) /*!< Background of the whole page */
#define SPLASH_COLOR_LOGO (STYLE_WHITE)       /*!< Logo colour */
#define SPLASH_COLOR_TITLE (STYLE_LIGHT_GRAY) /*!< First caption line (e.g. "FLASHING") */
#define SPLASH_COLOR_NAME (STYLE_WHITE)       /*!< Second caption line (e.g. the board name) */

/*!
 * \brief Return codes for splash operations.
 */
enum SplashReturnCode {
    SPLASH_RC_OK,           /*!< Operation completed successfully */
    SPLASH_RC_NULL_POINTER, /*!< A required pointer was NULL */
    SPLASH_RC_ERROR,        /*!< libraster or a draw callback failed, or the logo data is corrupt */
};

/*!
 * \brief Boxes of the splash page, top to bottom; together they cover the whole screen.
 */
enum SplashBoxId : uint8_t {
    SPLASH_BOX_LOGO,  /*!< Background of the logo (drawn on top by splash_api_draw_logo) */
    SPLASH_BOX_TITLE, /*!< First caption line */
    SPLASH_BOX_NAME,  /*!< Second caption line, down to the bottom of the screen */
    SPLASH_BOX_COUNT,
};

/*!
 * \brief Boxes and labels of the splash page, mounted as a libraster interface.
 */
struct SplashHandler {
    struct Label title;                 /*!< First caption line */
    struct Label name;                  /*!< Second caption line */
    struct Box boxes[SPLASH_BOX_COUNT]; /*!< Interface, indexed by SplashBoxId */
};

#endif // SPLASH_H
