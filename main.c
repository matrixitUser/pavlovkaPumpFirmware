#include "main.h"
#include "time.h"

#include "stdio.h"
#include "math.h"



PRIVATE void _WdgKick(u8 hdl, u32 time)
{
    IWDG_ReloadCounter();
    Bkp16_Write(BKP16_TIMEOFF, time >> 6);
    printf("\n\r%10d:+++ _WdgKick NA=%d mode=%d ++++\n\r", RTC_GetCounter(),g_sConfig.u8NetworkAddress, g_sConfig.u8Mode);
}

#ifdef __COUNTER_H__

PRIVATE void _HourlyWriteNewRecord(u8 hdl, u32 time)
{
	tsArchiveRecordPack record = Hourly_MakeRecord(time);
	Hourly_Write(&record);
}

PRIVATE void _AbnormalWriteResetFlags(uint32_t time)
{
	uint16_t resetFlags = Abnormal_GetResetFlags();
	tsAbnormalEvent ae = Abnormal_MakeRecord(time, AEID_RESTART_TYPE | resetFlags);
	Abnormal_Write(&ae);
}

PRIVATE void _AbnormalWriteTimeoff(uint32_t time)
{
	uint32_t timeMin = time >> 6;
	uint16_t lastMin = Bkp16_Read(BKP16_TIMEOFF);

	if(timeMin != lastMin)	// добавление события "отключение" в архив
	{
		u16 timeOff = 0xFFFF;
		if(timeMin > lastMin)
		{
			timeOff = timeMin - lastMin;
		}
		tsAbnormalEvent ae = Abnormal_MakeRecord(time, AEID_TIME_OFF_TYPE | timeOff);
		Abnormal_Write(&ae);
	}
}



PRIVATE void _BkpDomain_Recover(void)
{
	if(g_sConfig.u32ReleaseTs == TIMESTAMP_UNDEFINED) return;
	uint32_t time = Override_RTC_GetCounter(); 	// текущее время по версии RTC
	uint32_t flashTime = g_sConfig.u32ReleaseTs;	// дата релиза устройства

	tsArchiveRecordUnpack sLastHourly;			//последняя часовая запись
	sLastHourly.u32Ts = 0;

	if(Hourly_GetByOffset(&sLastHourly, 0) == SUCCESS)
	{
		if((sLastHourly.u32Ts != TIMESTAMP_UNDEFINED) && (sLastHourly.u32Ts > flashTime))
		{
			flashTime = sLastHourly.u32Ts;
		}
		else
		{
			sLastHourly.u32Ts = 0;
		}
	}

	if(time < flashTime)//TODO есть ли в этом необходимость и какие могут быть последствия?
	{
		tsAbnormalEvent ae;
		time = flashTime + 1;
		Rtc_SetTime(time);

		tsConfig sNewCfg;
		memcpy(&sNewCfg, &g_sConfig, sizeof(tsConfig));
		g_sConfig.u8IsRtcError = 1;

		if(sLastHourly.u32Ts != 0)
		{
			uint8_t i = 0;
			for(i = 0; i < COUNTERS; i++)
			{
				uint32_t u32Counter = Counter_GetPCntFromValue(i, sLastHourly.asCounter[i].u8Stat_u24Counter);
				sNewCfg.asChannel[i].u8IsError = 1;
				if(VBkp_Read(i) < u32Counter)
				{
					VBkp_Set(i, u32Counter);
				}
			}
			VBkp_mUpdate();
		}

		ae = Abnormal_MakeRecord(time, AEID_BKP_RECOVER_TYPE | (sLastHourly.u32Ts != 0));
		Abnormal_Write(&ae);

		Config_Set(&sNewCfg, RESET_SOFT);
	}

}

#endif

int main(void)
{
	typedef struct
	{
		u32 u32BaudRate;
		u8 u8WordLen;
		u8 u8StopBits;
		u8 u8Parity;
		u8 reserved;
	} tsUart;

#if defined(BOOTLOADER)			// при использовании бутлоадера
	__set_PRIMASK(1);
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, HW_FLASH_PSTART_ADDRESS);
	__set_PRIMASK(0);
	__enable_irq();
#endif

	DBGMCU_Config(DBGMCU_SLEEP | DBGMCU_STANDBY | DBGMCU_STOP | DBGMCU_IWDG_STOP, ENABLE); // для отладки
	//delay_us(2 * 1000 * 1000);

	IWDG_Init();					// побыстрее запускаем собаку
	SleepReset_Init(SLEEP_TYPE_WFI, 0, 0);
	IRQ_INIT(IRQ_GROUP);

	Clock_Init();					// запуск тактирования периферии; системная частота (HSI) 8 Мгц
	//GpioCtrl_InitDefaults();		// инициализация всех GPIO в Z
	//GpioCtrl_InitLeds();			// инициализация LED
	SigPwm_Init();					// иницилазация сигнального LED с ШИМ-модуляцией
	Adc1_Init();					// включение АЦП+ДМА

	Global_InitVars();				// инит глобальных переменных (g_*)
	Callback_InitVars();
	SWAlarm_InitVars();

	//uint8_t configOk = Config_Init();
	Config_Init();
#ifdef __COUNTER_H__
	Hourly_Init(configOk);
	Abnormal_Init(configOk);
#endif

	//uint8_t u8fRtcSet = 0;
	//SETBITS(u8fRtcSet, CHECK_BITS(g_sConfig.u16Status, CONFIG_STATUS_MODE_SERVICE)? RTC_INIT_FIRST : 0);
	//SETBITS(u8fRtcSet, CHECK_BITS(g_sConfig.u16Status, CONFIG_STATUS_RTC_INT)? RTC_INIT_LSI : 0);

	Rtc_Init();						// запуск часов; можно добавлять SWAlarm
	//VBkp_mInit();

	//Usart1_Subscribe(answerFrameModbusDataProcess);	//upp
	//Usart2_Subscribe(answerModbusUsart2DataProcess);	//wls
	//Usart3_Subscribe(FrameModbus_DataProcess);		//server

	Usart1_Subscribe(AnswerControllers);					//between ctrls
	Usart2_Subscribe(answerModbusUsart2DataProcess);		//wls
	//Usart3_Subscribe(FrameModbus_DataProcess);			//server gsm
	UART4_Subscribe(answerFrameModbusDataProcess);			//upp
	UART5_Subscribe(FrameModbus_DataProcess);				//server bis
	//GpioCtrl_TmrStatusLed_Init();


	//uint32_t time = Override_RTC_GetCounter();
	SWAlarm_AddInterval(20, 0, _WdgKick);				// пинаем собаку с интервалом (20сек)

	PWR_WakeUpPinCmd(DISABLE);

	Usart1_Init(&g_sConfig.sUart1);
	Usart2_Init(&g_sConfig.sUart2);
	Usart3_Init(&g_sConfig.sUart3);  //используем такие-же настройки порта как USART1  //отладка
	UART4_Init(&g_sConfig.sUart4);
	UART5_Init(&g_sConfig.sUart5);

	//gTimerForTalking = NETWORK_ADDRESS/10 + NETWORK_ADDRESS%10*1.5;
	printf("\n\r%d:RESTART NA=%d mode=%d\n\r", RTC_GetCounter(), g_sConfig.u8NetworkAddress, g_sConfig.u8Mode);
	master_init();

    while(1)
    {
        Callback_Action();
    	VBkp_mUpdate();
    	Sleep_Try();
    }
}
