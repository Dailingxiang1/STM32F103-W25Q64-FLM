/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright (c) 2012 Keil - An ARM Company. All rights reserved.     */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Device Description for                                */
/*               S25FL032P0XMFI010 populated on LPC4088-32 Dev. Kit    */
/*                                                                     */
/***********************************************************************/

// FlashDev.c
#include "FlashOS.H"

struct FlashDevice const FlashDevice  =
{
    FLASH_DRV_VERS,
    "W25Q64 STM32F103 SPI",
    EXTSPI,
    0x90000000,
    0x00800000,
    256,
    0,
    0xFF,
    100,
    3000,
    {{0x001000, 0x000000}, {SECTOR_END}}
};