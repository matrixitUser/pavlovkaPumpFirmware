#include "main.h"
#ifdef __COUNTER_H__

PUBLIC u32 Counter_GetPCnt(u8 n)
{
	if(n<COUNTERS)
	{
		return VBkp_Read(n);
	}
	return 0;
}

PUBLIC void Counter_SetPCnt(u8 n, u32 u32RegVal)
{
	if(n<COUNTERS)
	{
		VBkp_Set(n, u32RegVal);
	}
}

PUBLIC uint32_t Counter_GetValue(uint8_t n, tsChannelConfig *psChannelConfig)
{
	if(n >= COUNTERS || psChannelConfig == NULL) return 0;
	uint32_t temp = (VBkp_Read(n) / (1UL + psChannelConfig->u16KDiv)) & ARCHIVERECORD_COUNTER_VALUEMASK;
	uint32_t pins = GpioCtrl_GetPins();
	if(psChannelConfig->u8IsEnabled) temp |= ARCHIVERECORD_COUNTER_ENABLEMASK;
	if(psChannelConfig->u8IsError) temp |= ARCHIVERECORD_COUNTER_ERRORMASK;
	if(pins & (1UL << (n + 16))) temp |= ARCHIVERECORD_COUNTER_PIN1MASK;
	if(pins & (1UL << n)) temp |= ARCHIVERECORD_COUNTER_PIN0MASK;
	return temp;
}

PUBLIC void Counter_SetFromValue(uint8_t n, uint32_t value)
{
	if(n < COUNTERS)
	{
		VBkp_Set(n, (value & ARCHIVERECORD_COUNTER_VALUEMASK) * (1UL + g_sConfig.asChannel[n].u16KDiv));
	}
}

PUBLIC void Counter_Increment(u8 n, u8 bit)
{
	if((n < COUNTERS) && (bit == 0))
	{
		VBkp_Increment(n);
	}
}

PUBLIC uint32_t Counter_GetPCntFromValue(uint8_t n, uint32_t value)
{
	if(n < COUNTERS)
	{
		return (value & ARCHIVERECORD_COUNTER_VALUEMASK) * (1UL + g_sConfig.asChannel[n].u16KDiv);
	}
	return WORD_UNDEFINED;
}

#endif
