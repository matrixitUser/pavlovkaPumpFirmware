#ifndef __BKP16_H__
#define __BKP16_H__

#include "main.h"

#define BKP16_COUNT				10

#define BKP16_FLAG_RTC_STARTED	0x0001	// время началО идти - нужно ли проводить первичную инициализацию времени?
#define BKP16_FLAG_RTC_INT		0x0002	// тактирование от внутреннего генератора
#define BKP16_FLAG_RTC_LSE_BAD	0x0004	// показывает состояние запуска LSE-генератора

#define BKP_DR_START			BKP_DR1

//#define BKP16_VBKP0_1			0
//#define BKP16_VBKP2_3			1
//#define BKP16_VBKP4_5			2
//#define BKP16_VBKP6_7			3
//#define BKP16_VBKP8_9			4
//#define BKP16_VBKP10_11			5
//#define BKP16_VBKP12_13			6
//#define BKP16_VBKP14_15			7
#define BKP16_FLAGS				8  	//9
#define BKP16_TIMEOFF			9	//10

PUBLIC u16 	Bkp16_Read(u8 n);
PUBLIC void Bkp16_Write(u8 n, u16 value);
PUBLIC void Bkp16_ResetBits(uint8_t n, uint16_t bitMask);
PUBLIC void Bkp16_SetBits(uint8_t n, uint16_t bitMask);
PUBLIC uint16_t Bkp16_CheckBits(uint8_t n, uint16_t bitMask);
PUBLIC void Bkp16_ReadAll(uint16_t *outRead);
PUBLIC void Bkp16_WriteAll(uint16_t *inWrite);

#endif
