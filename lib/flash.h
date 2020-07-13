#ifndef __FLASH_H__
#define __FLASH_H__

#include "main.h"

PUBLIC void Flash_Protect(void);
PUBLIC void Flash_Read1(uint32_t addr, uint8_t *receiver, uint16_t length);
PUBLIC void Flash_Read2(uint32_t addr, uint16_t *receiver, uint16_t length);
PUBLIC void Flash_Write1(uint32_t addr, void *target, uint16_t length, uint8_t page);
PUBLIC void Flash_Write2(uint32_t addr, uint16_t *target, uint16_t length, uint8_t page);
PUBLIC void Flash_Write4(uint32_t addr, uint32_t *target, uint16_t length, uint8_t page);

#endif
