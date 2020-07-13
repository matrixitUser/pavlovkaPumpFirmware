#ifdef __COUNTER_H_   //попытка убрать функции регистратора
//#ifndef __COUNTER_H__

	#include "main.h"
	#ifdef __VBKP_H__

		#define __COUNTER_H__

		PUBLIC u32 Counter_GetPCnt(u8 n);
		PUBLIC void Counter_SetPCnt(u8 n, u32 u32RegVal);
		PUBLIC uint32_t Counter_GetValue(uint8_t n, tsChannelConfig *psChannelConfig);
		PUBLIC void Counter_SetFromValue(uint8_t n, uint32_t value);
		PUBLIC void Counter_Increment(u8 n, u8 bit);
		PUBLIC uint32_t Counter_GetPCntFromValue(uint8_t n, uint32_t value);

	#endif
#endif
