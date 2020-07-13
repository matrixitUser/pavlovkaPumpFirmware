#include "main.h"

PUBLIC void Flash_Protect(void)
{
    if (FLASH_GetReadOutProtectionStatus() == RESET)
    {
        FLASH_Unlock();
        FLASH_ReadOutProtection(ENABLE);
        FLASH_Lock();
    }
}

PUBLIC void Flash_Read1(uint32_t addr, uint8_t *receiver, uint16_t length)
{
	uint16_t i;
	for(i = 0; i < length; i++)
	{
		receiver[i] = *(uint8_t *)(addr + i);
	}
}

PUBLIC void Flash_Read2(uint32_t addr, uint16_t *receiver, uint16_t length)
{
	uint16_t i;
	for(i = 0; i < (length >> 1); i++)
	{
		receiver[i] = *(uint16_t*)(addr + (i << 1));
	}
}

PUBLIC void Flash_Write1(uint32_t addr, void *target, uint16_t length, uint8_t page)
{
	uint16_t i;
	uint8_t *pData = (uint8_t *)target;
	__disable_irq();
#ifdef __SLEEP_RESET_H__
	Reset_Config(RESET_FLAG_WFLASH, SET);
#endif

	FLASH_Unlock();
	if(page > 0)
	{
		FLASH_ErasePage(addr & 0xFFFFFC00);
	}
	for(i = 0; i < (length >> 1); i++)
	{
		uint16_t value = memget_u16(pData);
		FLASH_ProgramHalfWord(addr, value);
		addr += 2;
		pData += 2;
	}
	FLASH_Lock();

#ifdef __SLEEP_RESET_H__
	Reset_Config(RESET_FLAG_WFLASH, RESET);
#endif
	__enable_irq();
}

PUBLIC void Flash_Write2(uint32_t addr, uint16_t *target, uint16_t length, uint8_t page)
{
	uint16_t i;
	__disable_irq();
#ifdef __SLEEP_RESET_H__
	Reset_Config(RESET_FLAG_WFLASH, SET);
#endif

	FLASH_Unlock();
	if(page > 0)
	{
		FLASH_ErasePage(addr & 0xFFFFFC00);
	}
	for(i = 0; i < (length >> 1); i++)
	{
		FLASH_ProgramHalfWord(addr + (i << 1), target[i]);
	}
	FLASH_Lock();

#ifdef __SLEEP_RESET_H__
	Reset_Config(RESET_FLAG_WFLASH, RESET);
#endif
	__enable_irq();
}

PUBLIC void Flash_Write4(uint32_t addr, uint32_t *target, uint16_t length, uint8_t page)
{
	uint16_t i;
	__disable_irq();
#ifdef __SLEEP_RESET_H__
	Reset_Config(RESET_FLAG_WFLASH, SET);
#endif

	FLASH_Unlock();
	if(page > 0)
	{
		FLASH_ErasePage(addr & 0xFFFFFC00);
	}
	for(i =0 ; i < (length >> 2); i++)
	{
		FLASH_ProgramWord(addr + (i << 2), target[i]);
	}
	FLASH_Lock();

#ifdef __SLEEP_RESET_H__
	Reset_Config(RESET_FLAG_WFLASH, RESET);
#endif
	__enable_irq();
}

//// PRIVATE //// PRIVATE //// PRIVATE //// PRIVATE //// PRIVATE //// PRIVATE //// PRIVATE //// PRIVATE //// PRIVATE //// PRIVATE //// PRIVATE //// PRIVATE //// PRIVATE //// PRIVATE



