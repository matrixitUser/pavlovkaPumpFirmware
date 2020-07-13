#include "main.h"

#ifdef __SLEEP_RESET_H__

#ifndef GPIO_CPU_LED_ON
#define GPIO_CPU_LED_ON
#endif

#ifndef GPIO_CPU_LED_OFF
#define GPIO_CPU_LED_OFF
#endif

PRIVATE uint32_t _sleepFlags;
PRIVATE uint32_t _resetFlags;
PRIVATE uint8_t _reset;
PRIVATE uint8_t _sleepType;

PUBLIC void SleepReset_Init(uint8_t sleepType, uint32_t sleepFlags, uint32_t resetFlags)
{
	_sleepType = sleepType;
	_sleepFlags = sleepFlags;
	_resetFlags = resetFlags;
	_reset = 0;
}


PUBLIC void Sleep_Config(uint32_t __flags, uint8_t newState)
{
	if(newState>0){
		SETBITS(_sleepFlags, __flags);
	} else {
		CLRBITS(_sleepFlags, __flags);
	}
}


PUBLIC void Reset_Config(uint32_t __flags, uint8_t newState)
{
	if(newState>0){
		SETBITS(_resetFlags, __flags);
	} else {
		CLRBITS(_resetFlags, __flags);
	}
}


PUBLIC void Reset_Try(void)
{
	if(_reset && (_resetFlags == 0))
	{
		NVIC_SystemReset();
	}
}


PUBLIC void Sleep_Try(void)
{
	if(_reset && (_resetFlags == 0))
	{
		NVIC_SystemReset();
	}

	if(_sleepType != SLEEP_TYPE_NEVER)
	{
		if((_sleepFlags == 0) && (_sleepType == SLEEP_TYPE_STOP))
		{
			GPIO_CPU_LED_OFF;
			PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);
		}
		else
		{
			GPIO_CPU_LED_OFF;
			CLRBIT(SCB->SCR, SCB_SCR_SLEEPDEEP);
			__WFI();
		}
	}
	//GPIO_CPU_LED_ON;
}

PUBLIC void Reset_Soft(void)
{
	uint32_t time = Override_RTC_GetCounter();
    Bkp16_Write(BKP16_TIMEOFF, time >> 6);
	_reset = 1;
}

#endif
