/*!
 * \file splash-api.h
 * \date 2026-10-29
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 * \ingroup Core
 *
 * \brief Public API of the splash screen.
 */

#ifndef SPLASH_API_H
#define SPLASH_API_H

#include "font.h"
#include "raster.h"
#include "splash.h"

/*!
 * \brief Build the splash boxes, with an empty caption.
 *
 * \details Every box is flagged for redraw: mount handler->boxes as the
 *     raster interface and render, then call splash_api_draw_logo.
 *
 * \param handler The splash page to build.
 * \param font Font of both caption lines (the firmware and the bootloader
 *     use different glyph sets).
 *
 * \retval SPLASH_RC_OK on success.
 * \retval SPLASH_RC_NULL_POINTER if \p handler or \p font is NULL.
 * \retval SPLASH_RC_ERROR if libraster rejected a box or a label.
 */
enum SplashReturnCode splash_api_init(struct SplashHandler *handler, const struct Font *font);

/*!
 * \brief Set the two caption lines.
 *
 * \details Only a line whose text actually changes is flagged for redraw.
 *     The strings are referenced, not copied: they must outlive the page.
 *
 * \param handler The splash page.
 * \param title First line, NULL or "" for none.
 * \param name Second line, NULL or "" for none.
 *
 * \retval SPLASH_RC_OK on success.
 * \retval SPLASH_RC_NULL_POINTER if \p handler is NULL.
 * \retval SPLASH_RC_ERROR if libraster rejected a text.
 */
enum SplashReturnCode splash_api_set_caption(struct SplashHandler *handler, const char *title, const char *name);

/*!
 * \brief Draw the logo, on top of the logo box.
 *
 * \details Call it after every render that redrew SPLASH_BOX_LOGO: the box
 *     fill covers the logo. Uses the raster's mask path when it has one (the
 *     logo is decoded into its scratch, in bands of rows if it doesn't fit),
 *     one rectangle per run of equal coverage otherwise.
 *
 * \param raster The raster the splash page is mounted on.
 *
 * \retval SPLASH_RC_OK on success.
 * \retval SPLASH_RC_NULL_POINTER if \p raster or its draw callback is NULL.
 * \retval SPLASH_RC_ERROR if a draw callback failed or the logo data is corrupt.
 */
enum SplashReturnCode splash_api_draw_logo(const struct RasterHandler *raster);

#endif // SPLASH_API_H
