/*!
 * \file bootloader-screen.h
 * \date 2026-10-29
 * \authors Alessandro Bridi [ale.bridi15@gmail.com]
 *
 * \brief Bootloader screen: the firmware's splash page (logo), with
 *     "FLASHING STEERING WHEEL" under it while a flashing tool is connected.
 *
 * \details Drawn by the CPU straight into the LTDC framebuffer (single
 *     buffer): the page is drawn once and only the caption changes.
 */

#ifndef BOOTLOADER_SCREEN_H
#define BOOTLOADER_SCREEN_H

#include <stdbool.h>

/*!
 * \brief Draw the logo page.
 *
 * \details Call it after MX_FMC_Init and MX_LTDC_Init, before turning the
 *     backlight on so the random SDRAM content is never shown.
 *
 * \retval true on success.
 * \retval false if libraster or the splash page failed.
 */
bool bootloader_screen_init(void);

/*!
 * \brief Show or hide the caption.
 *
 * \details Cheap when nothing changes: call it on every main loop iteration.
 *
 * \param flashing Whether a flashing tool is connected (ComIsConnected()).
 */
void bootloader_screen_update(bool flashing);

#endif // BOOTLOADER_SCREEN_H
