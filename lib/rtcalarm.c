#include "main.h"

#ifdef __RTCALARM_H__

#if 0 //мудрёная инициализация
/*
 * Функция инициализации времени
 *
 * Вход функции
 * bool bLsi - выбор LSx: 0=>LSE; 1=>LSI
 *
 * Выход функции
 * uint8_t fRes - флаги результата
 *
 * Функция инициализирует выбранный LSx в случаях, если:
 * - это первая инициализация функции
 * - текущий LSx не совпадает с выбранным
 *
 * При инициализации происходит сохранение, очистка и восстановление BKP-регистров
 *
 */
PUBLIC uint8_t Rtc_Init(uint8_t u8fInit)
{
	uint8_t fRes = 0;
	PWR_BackupAccessCmd(ENABLE);

	uint32_t time = Override_RTC_GetCounter();
	// если LSx не инициализирован, то время равно нулю
	// чтобы исключить другие факторы, будем учитывать бит инициализации LSx
	uint8_t rtcInitOK = (CHECK_BITS(u8fInit, RTC_INIT_FIRST) == 0) && (time > 0) && (Bkp16_CheckBits(BKP16_FLAGS, BKP16_FLAG_RTC_STARTED) > 0);
	// инициализация LSx зависит от выбранного генератора
	// ловим переход LSI->LSE и обратно
	uint8_t lsxReset = ((CHECK_BITS(u8fInit, RTC_INIT_LSI) > 0) != (Bkp16_CheckBits(BKP16_FLAGS, BKP16_FLAG_RTC_INT) > 0));

	if((rtcInitOK == 0) || (lsxReset > 0)) //инициализация RTC
	{
		uint16_t bkp16_save[BKP16_COUNT];

		// сохранение значений BKP-домена
		if(rtcInitOK > 0)
		{
			Bkp16_ReadAll(bkp16_save);
		}

		//if(rtcInitOK == 0)
		{
			//сброс BKP-домена
			BKP_DeInit();
		}

		// Инициализация LSx
		uint32_t rtcPrs;
		if(CHECK_BITS(u8fInit, RTC_INIT_LSI) == 0)
		{
			Bkp16_SetBits(BKP16_FLAGS, BKP16_FLAG_RTC_LSE_BAD);
			RCC_LSEConfig(RCC_LSE_ON);

			uint8_t u8LseTimeout = 0;
			while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)
			{
				delay_us(10000);
				if(++u8LseTimeout == 0) break;	//timeout ~2560ms
			}

			if(RCC_GetFlagStatus(RCC_FLAG_LSERDY) != RESET)
			{
				RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
				RCC_RTCCLKCmd(ENABLE);
				rtcPrs = 32768UL - 1;
				Bkp16_ResetBits(BKP16_FLAGS, BKP16_FLAG_RTC_INT | BKP16_FLAG_RTC_LSE_BAD);
			}
		}

		if((CHECK_BITS(u8fInit, RTC_INIT_LSI) > 0) || Bkp16_CheckBits(BKP16_FLAGS, BKP16_FLAG_RTC_LSE_BAD))
		{
			RCC_LSEConfig(RCC_LSE_OFF);
		    RCC_LSICmd(ENABLE);
			while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET) { }
			RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
			RCC_RTCCLKCmd(ENABLE);
			rtcPrs = 40000UL - 1;
			Bkp16_SetBits(BKP16_FLAGS, BKP16_FLAG_RTC_INT);
		}

		RTC_WaitForSynchro();
		RTC_WaitForLastTask();

		RTC_SetPrescaler(rtcPrs); // RTC period = RTCCLK/RTC_PR = (32.768 KHz)/(32767+1)
		RTC_WaitForLastTask();

		//
		Bkp16_SetBits(BKP16_FLAGS, BKP16_FLAG_RTC_STARTED);

		// состояние флагов поменялось, сохраняем перед восстановлением
		bkp16_save[BKP16_FLAGS] = Bkp16_Read(BKP16_FLAGS);

		// Восстановление BKP-домена
		if(rtcInitOK > 0)
		{
			Bkp16_WriteAll(bkp16_save);
			Rtc_SetTime(time + 1);
		}
	}
	else
	{
		RTC_WaitForSynchro();
	}

	RTC_ITConfig(RTC_IT_ALR, ENABLE);
	RTC_WaitForLastTask();

	// EXTI
	EXTI_InitTypeDef EXTI_InitStructure;

	//RTC Alarm
	EXTI_InitStructure.EXTI_Line = EXTI_Line17;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);
	//
	NVIC_EnableIRQ(RTCAlarm_IRQn);
	NVIC_SetPriority(RTCAlarm_IRQn, NVIC_PRIORITY_RTC);
	//

	//time = Override_RTC_GetCounter();
	//return time;

	return fRes;
}
#else
PUBLIC void Rtc_Init(void)
{
	// Разрешить доступ к области резервных данных
	PWR_BackupAccessCmd(ENABLE);

	// RTC
	uint32_t time = Override_RTC_GetCounter();
	if(time == 0) //время не идёт; инициализация RTC
	{
		//сброс BKP-домена
		BKP_DeInit();
	#ifdef LSI
	    RCC_LSICmd(ENABLE);
		while(RCC_GetFlagStatus(RCC_FLAG_LSIRDY) == RESET);
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSI);
		RCC_RTCCLKCmd(ENABLE);
	#else
		RCC_LSEConfig(RCC_LSE_ON);
		while(RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET);
		RCC_RTCCLKConfig(RCC_RTCCLKSource_LSE);
		RCC_RTCCLKCmd(ENABLE);
	#endif

		RTC_WaitForSynchro();
		RTC_WaitForLastTask();

		RTC_SetPrescaler(RTCCLK - 1);
		RTC_WaitForLastTask();
	}
	else
	{
		RTC_WaitForSynchro();
	}

	RTC_ITConfig(RTC_IT_ALR, ENABLE);
	RTC_WaitForLastTask();

	// EXTI
	EXTI_InitTypeDef EXTI_InitStructure;

	//RTC Alarm
	EXTI_InitStructure.EXTI_Line = EXTI_Line17;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = RTCAlarm_IRQn;
#ifdef CHIP_0XX
    NVIC_InitStructure.NVIC_IRQChannelPriority = NVIC_PRIORITY_RTC;
#else
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_PRIORITY_PREEMPTIVE(NVIC_PRIORITY_RTC);
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = IRQ_PRIORITY_NONPREEMPTIVE(NVIC_PRIORITY_RTC);
#endif
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}
#endif

//установка будильника на ближайшее время
//возвращает 1, если можно спать и 0 - если нельзя
uint8_t RtcAlarm_Reset(void)
{
	uint32_t time = Override_RTC_GetCounter();
	uint32_t newalarm = SWAlarm_Next(time);	//ставим время на ближайший будильник

	//if(newalarm <= time)
	//{
	//	SWAlarm_Tick();
	//	return 0;
	//}

	RTC_SetAlarm(newalarm);
	RTC_WaitForLastTask();

	return 1;
}

void Rtc_SetTime(uint32_t newtime)
{
	RTC_SetCounter(newtime);
	RTC_WaitForLastTask();

	IWDG_ReloadCounter();//!
    Bkp16_Write(BKP16_TIMEOFF, newtime >> 6);

	RtcAlarm_Reset();
}

void RTCALARM_IRQ()
{
	GPIO_CPU_LED_ON;
    if(RTC_GetITStatus(RTC_IT_ALR))
    {
    	SWAlarm_Tick();
		RTC_WaitForLastTask();
		RTC_ClearITPendingBit(RTC_IT_ALR);
		RTC_WaitForLastTask();
		EXTI_ClearITPendingBit(EXTI_Line17);
    }
}

#endif
