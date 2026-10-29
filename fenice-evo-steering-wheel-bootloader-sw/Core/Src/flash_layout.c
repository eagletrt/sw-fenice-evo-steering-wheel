/************************************************************************************/ /**
* \file         Core/Src/flash_layout.c
* \brief        Flash layout of the STM32H723ZG (single bank, 8 x 128 KB sectors).
* \note         #included by flash.c (BOOT_FLASH_CUSTOM_LAYOUT_ENABLE in blt_conf.h), so
*               it is excluded from the build in platformio.ini. OpenBLT's default
*               1024 KB table is for dual-bank parts (2 x 512 KB, the second bank at
*               0x08100000): on the H723 it would leave 0x08080000-0x080FFFFF out and
*               reject every firmware bigger than 384 KB.
****************************************************************************************/

/** \brief   Array with the layout of the flash memory.
 *  \details Sector 0 holds the bootloader (STM32H723ZGTx_FLASH.ld of the bootloader,
 *           STM32H723XG_FLASH_SHIFTED.ld of the firmware): keep it out of this table.
 */
static const tFlashSector flashLayout[] = {
    /* { 0x08000000, 0x20000, 0, 1 },     flash sector 0 - reserved for the bootloader  */
    { 0x08020000, 0x20000, 1, 1 }, /* flash sector 1 - 128 KB                        */
    { 0x08040000, 0x20000, 2, 1 }, /* flash sector 2 - 128 KB                        */
    { 0x08060000, 0x20000, 3, 1 }, /* flash sector 3 - 128 KB                        */
    { 0x08080000, 0x20000, 4, 1 }, /* flash sector 4 - 128 KB                        */
    { 0x080A0000, 0x20000, 5, 1 }, /* flash sector 5 - 128 KB                        */
    { 0x080C0000, 0x20000, 6, 1 }, /* flash sector 6 - 128 KB                        */
    { 0x080E0000, 0x20000, 7, 1 }, /* flash sector 7 - 128 KB                        */
};

/*********************************** end of flash_layout.c ****************************/
