
#include "gpioctrl.h"

PRIVATE uint8_t _u8tmrStatusState;
PRIVATE uint16_t _u16tmrStatusTimeout;
PRIVATE uint16_t _u16tmrStatusValue;
PRIVATE uint16_t _u16tmrStatusMask;

PUBLIC void GpioCtrl_TmrStatusLed_Init(void)
{
	_u16tmrStatusTimeout = 0;
	_u8tmrStatusState = 0;

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
	TIM_TimeBaseInitStruct.TIM_Prescaler = (8 - 1);				// every 1ms
	TIM_TimeBaseInitStruct.TIM_Period = (1000);					//
	TIM_TimeBaseInit(TIM16, &TIM_TimeBaseInitStruct);
	TIM_Cmd(TIM16, ENABLE);

	TIM_ITConfig(TIM16, TIM_IT_Update, ENABLE);

	NVIC_EnableIRQ(TIM1_UP_TIM16_IRQn);
	NVIC_SetPriority(TIM1_UP_TIM16_IRQn, NVIC_PRIORITY_TIMER);
}


void TMR_STATUS_LED_IRQ()
{

	GPIO_CPU_LED_ON;
	if(TIM_GetITStatus(TIM16, TIM_IT_Update))
	{
		if(_u16tmrStatusTimeout == 0)
		{
			switch(_u8tmrStatusState)
			{
			case 0:
				//GPIO_STATUS_LED_OFF;
				_u16tmrStatusTimeout = 3000;
				_u16tmrStatusValue = g_sConfig.u8Mode;
				_u8tmrStatusState = 1;

				for(_u16tmrStatusMask = 0x80; _u16tmrStatusMask; _u16tmrStatusMask >>= 1)
				{
					if(_u16tmrStatusValue & _u16tmrStatusMask) break;
				}
				break;

			//мигание бита статуса
			case 1:
				//GPIO_STATUS_LED_ON;
				_u16tmrStatusTimeout = (_u16tmrStatusValue & _u16tmrStatusMask)? 500 : 100;
				_u8tmrStatusState = 2;
				break;

			//сдвиг маски статуса
			case 2:
				//GPIO_STATUS_LED_OFF;
				_u16tmrStatusTimeout = 500;
				_u16tmrStatusMask >>= 1;
				_u8tmrStatusState = _u16tmrStatusMask? 1 : 0;
				break;

			default:
				_u16tmrStatusTimeout = 0;
				_u8tmrStatusState = 0;
				break;
			}

		}
		else
		{
			_u16tmrStatusTimeout--;
		}
		TIM_ClearITPendingBit(TIM16, TIM_IT_Update);
	}
}

PUBLIC void GpioCtrl_InitDefaults(void)
{
	// инициализация GPIO по умолчанию
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_Init(GPIOC, &GPIO_InitStructure);
    GPIO_Init(GPIOD, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

PUBLIC void GpioCtrl_InitLeds(void)
{
    //инициализация LED
    GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
#if (defined(GPIO_CPU_LED_PORT) && defined(GPIO_CPU_LED_PIN))
    GPIO_InitStructure.GPIO_Pin   = GPIO_CPU_LED_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(GPIO_CPU_LED_PORT, &GPIO_InitStructure);
#endif

    GPIO_CPU_LED_ON;

#if (defined(GPIO_STATUS_LED_PORT) && defined(GPIO_STATUS_LED_PIN))
    GPIO_InitStructure.GPIO_Pin   = GPIO_STATUS_LED_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(GPIO_STATUS_LED_PORT, &GPIO_InitStructure);
#endif
    GPIO_STATUS_LED_OFF;

#if (defined(GPIO_SIG_LED_PORT) && defined(GPIO_SIG_LED_PIN))
    GPIO_InitStructure.GPIO_Pin   = GPIO_SIG_LED_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(GPIO_SIG_LED_PORT, &GPIO_InitStructure);
#endif
    GPIO_SIG_LED_OFF;
}

/////////
#if defined(__GPIOCTRL_H__)

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define FILTERMAX		8

PRIVATE uint8_t _filterCount = 0;
PRIVATE uint8_t _au8Filter[32];				// фильтр по 32 входам

PRIVATE uint8_t _curPortIOIsInit;
PRIVATE uint32_t _curPortIOFiltered;		// текущие отфильтрованные значения GPIO

// распиновка счётных входов простая: все GPIOB от 0 до 15
// распиновка магнитных входов:
#if (VERSION == 1)
PRIVATE uint16_t _CDIO[32] = {
	//GPIOB
	TO_UINT16(GPIO_PortSourceGPIOB, 0),
	TO_UINT16(GPIO_PortSourceGPIOB, 1),
	TO_UINT16(GPIO_PortSourceGPIOB, 2),
	TO_UINT16(GPIO_PortSourceGPIOB, 3),
	TO_UINT16(GPIO_PortSourceGPIOB, 4),
	TO_UINT16(GPIO_PortSourceGPIOB, 5),
	TO_UINT16(GPIO_PortSourceGPIOB, 6),
	TO_UINT16(GPIO_PortSourceGPIOB, 7),
	TO_UINT16(GPIO_PortSourceGPIOB, 8),
	TO_UINT16(GPIO_PortSourceGPIOB, 9),
	TO_UINT16(GPIO_PortSourceGPIOB, 10),
	TO_UINT16(GPIO_PortSourceGPIOB, 11),
	TO_UINT16(GPIO_PortSourceGPIOB, 12),
	TO_UINT16(GPIO_PortSourceGPIOB, 13),
	TO_UINT16(GPIO_PortSourceGPIOB, 14),
	TO_UINT16(GPIO_PortSourceGPIOB, 15),

	//GPIOA
	TO_UINT16(GPIO_PortSourceGPIOA, 8),
	TO_UINT16(GPIO_PortSourceGPIOA, 11),
	TO_UINT16(GPIO_PortSourceGPIOA, 15),
	//GPIOC
	TO_UINT16(GPIO_PortSourceGPIOC, 10),
	TO_UINT16(GPIO_PortSourceGPIOC, 11),
	TO_UINT16(GPIO_PortSourceGPIOC, 12),
	//GPIOD
	TO_UINT16(GPIO_PortSourceGPIOD, 2),
	//GPIOC
	TO_UINT16(GPIO_PortSourceGPIOC, 13),
	//GPIOA
	TO_UINT16(GPIO_PortSourceGPIOA, 6),
	TO_UINT16(GPIO_PortSourceGPIOA, 7),
	//GPIOA
	TO_UINT16(GPIO_PortSourceGPIOC, 4),
	TO_UINT16(GPIO_PortSourceGPIOC, 5),
	TO_UINT16(GPIO_PortSourceGPIOC, 6),
	TO_UINT16(GPIO_PortSourceGPIOC, 7),
	TO_UINT16(GPIO_PortSourceGPIOC, 8),
	TO_UINT16(GPIO_PortSourceGPIOC, 9)
};
#elif (VERSION == 2)
//электрические
PRIVATE uint16_t _CDIO[32] = {
	// счетные
	// СТ5
		TO_UINT16(GPIO_PortSourceGPIOB, 8),
		TO_UINT16(GPIO_PortSourceGPIOA, 6),
		TO_UINT16(GPIO_PortSourceGPIOB, 9),
		TO_UINT16(GPIO_PortSourceGPIOA, 7),
	// СТ6
		TO_UINT16(GPIO_PortSourceGPIOB, 10),
		TO_UINT16(GPIO_PortSourceGPIOC, 4),
		TO_UINT16(GPIO_PortSourceGPIOB, 11),
		TO_UINT16(GPIO_PortSourceGPIOC, 5),
	// СТ7
		TO_UINT16(GPIO_PortSourceGPIOB, 12),
		TO_UINT16(GPIO_PortSourceGPIOC, 6),
		TO_UINT16(GPIO_PortSourceGPIOB, 13),
		TO_UINT16(GPIO_PortSourceGPIOC, 7),
	// СТ8
		TO_UINT16(GPIO_PortSourceGPIOB, 14),
		TO_UINT16(GPIO_PortSourceGPIOC, 8),
		TO_UINT16(GPIO_PortSourceGPIOB, 15),
		TO_UINT16(GPIO_PortSourceGPIOC, 9),

	// магнитные
	// СТ1
		TO_UINT16(GPIO_PortSourceGPIOB, 0),
		TO_UINT16(GPIO_PortSourceGPIOA, 8),
		TO_UINT16(GPIO_PortSourceGPIOB, 1),
		TO_UINT16(GPIO_PortSourceGPIOA, 11),
	// СТ2
		TO_UINT16(GPIO_PortSourceGPIOB, 2),
		TO_UINT16(GPIO_PortSourceGPIOA, 15),
		TO_UINT16(GPIO_PortSourceGPIOB, 3),
		TO_UINT16(GPIO_PortSourceGPIOC, 10),
	// СТ3
		TO_UINT16(GPIO_PortSourceGPIOB, 4),
		TO_UINT16(GPIO_PortSourceGPIOC, 11),
		TO_UINT16(GPIO_PortSourceGPIOB, 5),
		TO_UINT16(GPIO_PortSourceGPIOC, 12),
	// СТ4
		TO_UINT16(GPIO_PortSourceGPIOB, 6),
		TO_UINT16(GPIO_PortSourceGPIOD, 2),
		TO_UINT16(GPIO_PortSourceGPIOB, 7),
		TO_UINT16(GPIO_PortSourceGPIOC, 13)
};
#elif (VERSION == 3)
//электрические
PRIVATE uint16_t _CDIO[32] = {
	// счетные
		TO_UINT16(GPIO_PortSourceGPIOB, 6),	//СТ4#1
		TO_UINT16(GPIO_PortSourceGPIOD, 2),	//CT4#2
		TO_UINT16(GPIO_PortSourceGPIOB, 7),	//CT4#3

		TO_UINT16(GPIO_PortSourceGPIOB, 8),	//CT5#1
		TO_UINT16(GPIO_PortSourceGPIOA, 6),	//CT5#2
		TO_UINT16(GPIO_PortSourceGPIOB, 9),	//CT5#3

		TO_UINT16(GPIO_PortSourceGPIOB, 10),//CT6#1
		TO_UINT16(GPIO_PortSourceGPIOC, 4),	//CT6#2
		TO_UINT16(GPIO_PortSourceGPIOB, 11),//CT6#3

		TO_UINT16(GPIO_PortSourceGPIOB, 12),//CT7#1
		TO_UINT16(GPIO_PortSourceGPIOC, 6),	//CT7#2
		TO_UINT16(GPIO_PortSourceGPIOB, 13),//CT7#3

		TO_UINT16(GPIO_PortSourceGPIOB, 14),//CT8#1
		TO_UINT16(GPIO_PortSourceGPIOC, 8),	//CT8#2
		TO_UINT16(GPIO_PortSourceGPIOB, 15),//CT8#3

		TO_UINT16(GPIO_PortSourceGPIOC, 9),	//CT8#4

	// магнитные
		TO_UINT16(GPIO_PortSourceGPIOB, 0),	//CT1#1
		TO_UINT16(GPIO_PortSourceGPIOA, 8),	//CT1#2
		TO_UINT16(GPIO_PortSourceGPIOB, 1),	//CT1#3
		TO_UINT16(GPIO_PortSourceGPIOA, 11),//CT1#4

		TO_UINT16(GPIO_PortSourceGPIOB, 2),	//CT2#1
		TO_UINT16(GPIO_PortSourceGPIOA, 15),//CT2#2
		TO_UINT16(GPIO_PortSourceGPIOB, 3),	//CT2#3
		TO_UINT16(GPIO_PortSourceGPIOC, 10),//CT2#4

		TO_UINT16(GPIO_PortSourceGPIOB, 4),	//CT3#1
		TO_UINT16(GPIO_PortSourceGPIOC, 11),//CT3#2
		TO_UINT16(GPIO_PortSourceGPIOB, 5),	//CT3#3
		TO_UINT16(GPIO_PortSourceGPIOC, 12),//CT3#4

		TO_UINT16(GPIO_PortSourceGPIOC, 13),//CT4#4

		TO_UINT16(GPIO_PortSourceGPIOA, 7),	//CT5#4

		TO_UINT16(GPIO_PortSourceGPIOC, 5),	//CT6#4

		TO_UINT16(GPIO_PortSourceGPIOC, 7)	//CT7#4
};
#elif (VERSION == 6) && defined(__COUNTER_H)

//электрические
PRIVATE uint16_t _CDIO[32] = {
	// счетные
	// СТ5
		TO_UINT16(GPIO_PortSourceGPIOB, 8),
		TO_UINT16(GPIO_PortSourceGPIOA, 6),
		TO_UINT16(GPIO_PortSourceGPIOB, 9),
		TO_UINT16(GPIO_PortSourceGPIOA, 7),
	// СТ6
		TO_UINT16(GPIO_PortSourceGPIOB, 10),
		TO_UINT16(GPIO_PortSourceGPIOC, 4),
		TO_UINT16(GPIO_PortSourceGPIOB, 11),
		TO_UINT16(GPIO_PortSourceGPIOC, 5),
	// СТ7
		TO_UINT16(GPIO_PortSourceGPIOB, 12),
		TO_UINT16(GPIO_PortSourceGPIOC, 6),
		TO_UINT16(GPIO_PortSourceGPIOB, 13),
		TO_UINT16(GPIO_PortSourceGPIOC, 7),
	// СТ8
		TO_UINT16(GPIO_PortSourceGPIOB, 14),
		TO_UINT16(GPIO_PortSourceGPIOC, 8),
		TO_UINT16(GPIO_PortSourceGPIOB, 15),
		TO_UINT16(GPIO_PortSourceGPIOC, 9),

	// магнитные
	// СТ1
		TO_UINT16(GPIO_PortSourceGPIOB, 0),
		TO_UINT16(GPIO_PortSourceGPIOA, 8),
		TO_UINT16(GPIO_PortSourceGPIOB, 1),
		TO_UINT16(GPIO_PortSourceGPIOA, 11),
	// СТ2
		TO_UINT16(GPIO_PortSourceGPIOB, 2),
		TO_UINT16(GPIO_PortSourceGPIOA, 15),
		TO_UINT16(GPIO_PortSourceGPIOB, 3),
		TO_UINT16(GPIO_PortSourceGPIOC, 10),
	// СТ3
		TO_UINT16(GPIO_PortSourceGPIOB, 4),
		TO_UINT16(GPIO_PortSourceGPIOC, 11),
		TO_UINT16(GPIO_PortSourceGPIOB, 5),
		TO_UINT16(GPIO_PortSourceGPIOC, 12),
	// СТ4
		TO_UINT16(GPIO_PortSourceGPIOB, 6),
		TO_UINT16(GPIO_PortSourceGPIOD, 2),
		TO_UINT16(GPIO_PortSourceGPIOB, 7),
		TO_UINT16(GPIO_PortSourceGPIOC, 13)
};



PUBLIC void TMRFILTER_IRQ()
{
	GPIO_CPU_LED_ON;
	if(TIM_GetITStatus(TIM7, TIM_IT_Update))
	{
		volatile uint8_t i;
		uint32_t portValue[4];

		portValue[0] = GPIO_ReadInputData(GPIOA);
		portValue[1] = GPIO_ReadInputData(GPIOB);
		portValue[2] = GPIO_ReadInputData(GPIOC);
		portValue[3] = GPIO_ReadInputData(GPIOD);

		// фильтр пинов
		/*for(i = 0; i < 16; i++)
		{
			if(CHECK_BIT(portValue[1], i)) //GPIOB
			{
				_au8Filter[i]++;
			}
		}*/

		// накопление данных для фильтра
		for(i = 0; i < 32; i++)
		{
			if(CHECK_BIT(portValue[LOW(_CDIO[i])], HIGH(_CDIO[i])))
			{
				_au8Filter[i]++;
			}
		}

		//

		if(++_filterCount == FILTERMAX)
		{
			uint32_t tmpPortIO = _curPortIOFiltered;
			for(i = 0; i < 32; i++)
			{
				if(_au8Filter[i] > (FILTERMAX >> 1))
				{
					SETBIT(tmpPortIO, i);
				}
				else if(_au8Filter[i] < (FILTERMAX >> 1))
				{
					CLRBIT(tmpPortIO, i);
				}
				else
				{
					//no action
				}
				_au8Filter[i] = 0;
			}
			_filterCount = 0;

			//uint32_t changedDIO = tmpPortIO ^ _curPortIOFiltered;
			if(_curPortIOIsInit)
			{
				uint32_t mask;
				for(i = 0, mask = 0x00000001UL; i < COUNTERS; i++, mask <<= 1)
				{
					if(CHECK_BITS(_curPortIOFiltered, mask) != CHECK_BITS(tmpPortIO, mask))
					{
						Counter_Increment(i, (CHECK_BITS(tmpPortIO, mask) > 0));
					}
				}
			}
			_curPortIOFiltered = tmpPortIO;
			_curPortIOIsInit = 1;
		}

		//

		TIM_ClearITPendingBit(TIM7, TIM_IT_Update);
	}
}


PUBLIC void GpioCtrl_TmrFilter_Init(uint16_t tmrPeriod_msm1)
{
    GPIO_InitTypeDef GPIO_InitStructure;

	//A6..A8,A11,A15
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_11 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

	//B0..B15
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_All;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

	//C4..C13
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8
    		| GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

	//D2
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    //

	memset(_au8Filter, 0, sizeof(_au8Filter));
	_curPortIOFiltered = 0xFFFFFFFFUL;
	_curPortIOIsInit = 0;

	//

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
	TIM_TimeBaseInitStruct.TIM_Prescaler = 1000;				//8MHz / 1000 = 8000Hz or "8 times in 1ms"
	TIM_TimeBaseInitStruct.TIM_Period = tmrPeriod_msm1 + 1;		//period = Xms * 8
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseInitStruct);
	TIM_Cmd(TIM7, ENABLE);

	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
#ifdef CHIP_0XX
    NVIC_InitStructure.NVIC_IRQChannelPriority = NVIC_PRIORITY_TIMER;
#else
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_PRIORITY_PREEMPTIVE(NVIC_PRIORITY_TIMER);
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = IRQ_PRIORITY_NONPREEMPTIVE(NVIC_PRIORITY_TIMER);
#endif
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

PUBLIC uint32_t GpioCtrl_GetPins(void)
{
	return _curPortIOFiltered;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PRIVATE uint8_t _dTime;
PRIVATE uint32_t _lastTime;
PRIVATE uint32_t _curTime;
PRIVATE uint32_t _trmCheckErrorPeriod;

void TMRRTCCHECK_IRQ()
{
	GPIO_CPU_LED_ON;
	if(TIM_GetITStatus(TIM15, TIM_IT_Update))
	{
		/*if(_curTime != TIMESTAMP_UNDEFINED)
		{
			_lastTime = _curTime;
		}
		_curTime = Override_RTC_GetCounter();
		if((_lastTime != TIMESTAMP_UNDEFINED) && (_curTime != TIMESTAMP_UNDEFINED))
		{
			uint32_t dT = _curTime - _lastTime;
			_dTime = (dT > 0xFE)? 0xFE : dT;
		}
		else
		{
			_dTime = 0xFF;
		}

		if(_dTime != 0xFF)
		{
			if((_dTime < 5) || (_dTime > 20))
			{
				if(_trmCheckErrorPeriod >= 2)
				{
					if(CHECK_BITS(g_sConfig.u16Status, CONFIG_STATUS_RTC_INT) == 0)
					{
						tsConfig sNewCfg;
						memcpy(&sNewCfg, &g_sConfig, sizeof(tsConfig));
						SETBITS(sNewCfg.u16Status, CONFIG_STATUS_RTC_INT | CONFIG_STATUS_RTC_CN);

						Config_Set(&sNewCfg, RESET_SOFT);
					}
				}
				else
				{
					_trmCheckErrorPeriod++;
				}
			}
			else
			{
				_trmCheckErrorPeriod = 0;
			}
		}
		else
		{
			_trmCheckErrorPeriod = 0;
			_lastTime = TIMESTAMP_UNDEFINED;
		}*/
		TIM_ClearITPendingBit(TIM15, TIM_IT_Update);
	}
}

PUBLIC uint8_t GpioCtrl_TmrRtcCheck_GetDTime(void)
{
	return _dTime;
}

PUBLIC void GpioCtrl_TmrRtcCheck_Init(void)
{
	_trmCheckErrorPeriod = 0;
	_curTime = TIMESTAMP_UNDEFINED;
	_lastTime = TIMESTAMP_UNDEFINED;
	_dTime = 0xFF;

	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	TIM_TimeBaseStructInit(&TIM_TimeBaseInitStruct);
	TIM_TimeBaseInitStruct.TIM_Prescaler = (8000 - 1);			// 8MHz / 8000 = 1kHz or period = 1ms
	TIM_TimeBaseInitStruct.TIM_Period = (10 * 1000);			// ms to check => 10sec
	TIM_TimeBaseInit(TIM15, &TIM_TimeBaseInitStruct);
	TIM_Cmd(TIM15, ENABLE);

	TIM_ITConfig(TIM15, TIM_IT_Update, ENABLE);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_BRK_TIM15_IRQn;
#ifdef CHIP_0XX
    NVIC_InitStructure.NVIC_IRQChannelPriority = NVIC_PRIORITY_TIMER;
#else
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_PRIORITY_PREEMPTIVE(NVIC_PRIORITY_TIMER);
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = IRQ_PRIORITY_NONPREEMPTIVE(NVIC_PRIORITY_TIMER);
#endif
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#endif
#endif
