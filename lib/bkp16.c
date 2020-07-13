#include "bkp16.h"

PUBLIC u16 Bkp16_Read(u8 n)
{
	return BKP_ReadBackupRegister(BKP_DR_START + (n<<2));
}

PUBLIC void Bkp16_Write(u8 n, u16 value)
{
	BKP_WriteBackupRegister(BKP_DR_START + (n<<2), value);
}


PUBLIC void Bkp16_ResetBits(uint8_t n, uint16_t bitMask)
{
	uint16_t temp = BKP_ReadBackupRegister(BKP_DR_START + (n<<2));
	CLRBITS(temp, bitMask);
	BKP_WriteBackupRegister(BKP_DR_START + (n<<2), temp);
}

PUBLIC void Bkp16_SetBits(uint8_t n, uint16_t bitMask)
{
	uint16_t temp = BKP_ReadBackupRegister(BKP_DR_START + (n<<2));
	SETBITS(temp, bitMask);
	BKP_WriteBackupRegister(BKP_DR_START + (n<<2), temp);
}

PUBLIC uint16_t Bkp16_CheckBits(uint8_t n, uint16_t bitMask)
{
	uint16_t temp = BKP_ReadBackupRegister(BKP_DR_START + (n<<2));
	return CHECK_BITS(temp, bitMask);
}


PUBLIC void Bkp16_ReadAll(uint16_t *outRead)
{
	uint8_t i;
	for(i = 0; i < BKP16_COUNT; i++)
	{
		outRead[i] = BKP_ReadBackupRegister(BKP_DR_START + (i<<2));
	}
}

PUBLIC void Bkp16_WriteAll(uint16_t *inWrite)
{
	uint8_t i;
	for(i = 0; i < BKP16_COUNT; i++)
	{
		BKP_WriteBackupRegister(BKP_DR_START + (i<<2), inWrite[i]);
	}
}
