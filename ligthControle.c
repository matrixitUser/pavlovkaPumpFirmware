#ifdef LIGHT_CONTROLE
#include "ligthControle.h"
#include "time.h"

static uint16_t errorCode = 0;
static uint16_t eventCode = 0;
static uint8_t tacts = 0;
static uint8_t tactsEvent = 0;
static uint8_t iArchive = 0;
static u32 manual_begin; //время начала работы в режим ручного управления
static u32 HandModeStart1; //время начала работы в режим ручного управления канал 1
static u32 HandModeStart2; //время начала работы в режим ручного управления канал 2
static u32 HandModeStart3; //время начала работы в режим ручного управления канал 3
static u32 HandModeStart4; //время начала работы в режим ручного управления канал 4




static tryes = 0;  //реальное количество отправок сообщения (попыток, если контролируем состояние порта)
static tryesEvent = 0;  //реальное количество отправок сообщения (попыток, если контролируем состояние порта)
static uint32_t timeErrorMessage = 0;
static uint32_t timeEventMessage = 0;
static uint8_t fullNA;
static uint8_t turn=0;
static isSignal = 0;

void controlHandMode();

double ourAbs(double indouble)
{
	if(indouble<0) indouble=-indouble;
	return indouble;
}

void ligthGPIO_init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

  //OUT
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Pin   =  GPIO_PIN_LIGHT2 | GPIO_PIN_LIGHT3 | GPIO_PIN_LIGHT4 | GPIO_PIN_12V_OUT5 | GPIO_PIN_12V_OUT6; //Выходы реле
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Pin   =  GPIO_PIN_LIGHT1; //Выходы реле
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  setOutPins();

  //IN
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPD;
  GPIO_InitStructure.GPIO_Pin   = GPIO_PIN_CONTACTOR_IN1 | GPIO_PIN_CONTACTOR_IN2 | GPIO_PIN_CONTACTOR_IN3 | GPIO_PIN_CONTACTOR_IN4 | GPIO_PIN_12V_IN5; //Входа 220v AIN1
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPD;
  GPIO_InitStructure.GPIO_Pin   = GPIO_PIN_12V_IN6; //Входа 220v AIN1
  GPIO_Init(GPIOA, &GPIO_InitStructure);
}

void setOutPins()
{
	GPIO_ResetBits(GPIOB, GPIO_PIN_LIGHT2 | GPIO_PIN_LIGHT3 | GPIO_PIN_LIGHT4 | GPIO_PIN_12V_OUT5 | GPIO_PIN_12V_OUT6);
	GPIO_ResetBits(GPIOA, GPIO_PIN_LIGHT1);
}

void ligthControl_init(void)
{
	//g_sConfig.u8ControlMode = CONTROLE_BY_ASTR_TIMER;
  g_debug = 1;
  g_timeout = 0;

  ligthGPIO_init();
  /*
  g_stateLight = GPIO_ReadOutputDataBit(GPIO_PORT_LIGHT, GPIO_PIN_LIGHT);
  set_ligth(g_stateLight);
  */

  SWAlarm_AddInterval(MONITORING_TIME, 0, alarm5sec);

  SWAlarm_AddInterval(60, 0, ligthControle);


}

void ligthControle(u8 hdl, u32 time)
{
	uint8_t isAstr_timer =0;
	uint8_t isShedulle =0;
	uint8_t iChannel;
    for(iChannel=1;iChannel<=4;iChannel++)
    {
		if( (g_sControlMode[iChannel] == CONTROLE_BY_ASTR_TIMER) || (g_sControlMode[iChannel] == CONTROLE_BY_ASTR_SHEDULLE) )
		{
			isAstr_timer = 1;
		}
		if( (g_sControlMode[iChannel] == CONTROLE_BY_SHEDULLE) || (g_sControlMode[iChannel] == CONTROLE_BY_ASTR_SHEDULLE) )
		{
			isShedulle = 1;
		}

    }
    g_astrLight = -1;
	if(isAstr_timer) astr_timer(hdl, time);
	if(isShedulle) shedulle(hdl, time);
}

void alarm5sec(u8 hdl, u32 time)
{
	monitoring(hdl,time);
	photosensorCheck(hdl,time);
	turn=1-turn; if(turn)  event(); else error();  //в разных тактах вызывается производится посылка ошибок и событий
	signaling();
}



void signaling()
{
	if(GPIO_ReadInputDataBit(GPIO_PORT_SIGNALING, GPIO_PIN_SIGNALING))
	{
		g_errorCode = 0x100;
		isSignal = 1;

	}
	else
	{
		if(isSignal)
		{
			g_errorCode = (0x1000);// || (g_errorCode & 0xFF);
			isSignal = 0;
		}
	}
}

void astr_timer(u8 hdl, u32 time)
{

	double sunRiseGMT;double sunSetGMT;

	long ltime = (long) time;
	struct tm* currenTime = gmtime(&ltime);

	int   year = currenTime->tm_year+1900;
	int   month = currenTime->tm_mon +1;
	int   day = currenTime->tm_mday;
	if(year < 2019) //часы не установлены или сброшены, попытаемся  перевести в режим по фотодатчику
	{
		return;
	}

    calc_astr_time(time);
    uint8_t iChannel;
    for(iChannel=1;iChannel;iChannel++)
    {
		if( (g_sControlMode[iChannel] == CONTROLE_BY_ASTR_TIMER) || (g_sControlMode[iChannel] == CONTROLE_BY_ASTR_SHEDULLE) )
        {
        	{
        		g_astrLight = set_by_astr_time(time,iChannel);
        		set_ligthByChannel(iChannel, g_astrLight);
        	}
        }
    }

}

//срабатывание будильника
void shedulle(u8 hdl, u32 time)
{

	long ltime = (long) time;
	long itime =  gmtime(&itime);
	uint8_t u8Action = g_stateLight;
	uint8_t onOff;

	struct tm* currenTime = gmtime(&ltime);
	int currenTimeInSec  = 60*((currenTime -> tm_hour)*60 + currenTime -> tm_min);  //текущее время в секундах от начала суток
	uint8_t iChannel=0;
	volatile uint8_t iShedulle=0;
    for (iChannel = 1; iChannel <=4 ; iChannel++) //И каналы и расписания пронумерованы с 1
    {
    	if( (g_sControlMode[iChannel]  == CONTROLE_BY_SHEDULLE) || (g_sControlMode[iChannel]  == CONTROLE_BY_ASTR_SHEDULLE ) )
    	{
            for (iShedulle = 1; iShedulle <=2 ; iShedulle++)
            {
            	if((g_sOn[iChannel][iShedulle] != TIMESTAMP_UNDEFINED ) && (g_sOff[iChannel][iShedulle] != TIMESTAMP_UNDEFINED ))
            	{

             		long OnSec  = g_sOn[iChannel][iShedulle];   //время включения в минутах от начала суток
            		long OffSec = g_sOff[iChannel][iShedulle];  //время выключения в минутах от начала суток

            		//if(OffSec < OnSec) OffSec = OffSec +24*3600;
            		if(OffSec > OnSec)
            		{
                   		if( (OnSec <= currenTimeInSec)	&& 	(currenTimeInSec <= OffSec) )
                   		{
                   			onOff = 1;
                    	}
                   		else
                   		{
                   			onOff = 0;
                   		}
            		}else //Расписание не учитывает переход дня.Обычно время в минутах выкл меньше времени включения, если так
            		{
                   		if( (OffSec <= currenTimeInSec)	&& 	(currenTimeInSec <= OnSec) )
                   		{
                  			onOff = 0;

                    	}
                  		else
                   		{
                  			onOff = 1;
                   		}

					}
                	//Если астрон.таймер говорит что светло, то освещение не будет включено
            	    if( (onOff) && (g_sControlMode[iChannel]  == CONTROLE_BY_ASTR_SHEDULLE)  )
            	    {
            	    	if( g_astrLight != -1)
            	    		onOff = g_astrLight;
            	    }
                    set_ligthByChannel(iChannel,onOff);

            	}
        	}
    	}

	}
    g_astrLight = -1;
}

void set_ligthByChannel(uint8_t nChannel, uint8_t onOff)
{
	uint8_t u8Action = g_stateLight;
	if(onOff)
	{
		SETBIT(u8Action,nChannel-1);
	}
	else
	{
		CLRBIT(u8Action,nChannel-1);
	}
	set_ligth(u8Action);
}

//вкл/выкл освещения
void set_ligth(uint8_t u8Action)
{
	//Bkp16_Write(BKP_REGISTER_STATE_LIGHT, u16Action);
	if(u8Action  != g_stateLight)
	{
		for( volatile uint8_t iChannel = 1; iChannel<=4; iChannel++)
		{
			volatile uint8_t u8ActionI = u8Action & (1<<(iChannel-1) );
			volatile stateLightI = g_stateLight & (1<<(iChannel-1));
			if( u8ActionI  != stateLightI )
			{
				switch (iChannel) {
					case 1:
						GPIO_WriteBit(GPIO_PORT_LIGHT1,GPIO_PIN_LIGHT1,u8ActionI);
						break;
					case 2:
						GPIO_WriteBit(GPIO_PORT_LIGHT2,GPIO_PIN_LIGHT2,u8ActionI);
						break;
					case 3:
						GPIO_WriteBit(GPIO_PORT_LIGHT3,GPIO_PIN_LIGHT3,u8ActionI);
						break;
					case 4:
						GPIO_WriteBit(GPIO_PORT_LIGHT4,GPIO_PIN_LIGHT4,u8ActionI);
						break;
					default:
						break;
					}
			}
		}
		g_stateLight = u8Action;
		toArchive(RTC_GetCounter(),u8Action<<8);
	}
}


void calc_astr_time(u32 time)
{
	double sunRiseGMT;double sunSetGMT;
	long ltime = (long) time;
	struct tm* currenTime = gmtime(&ltime);
	int   year = currenTime->tm_year+1900;
	int   month = currenTime->tm_mon +1;
	int   day = currenTime->tm_mday;

	sunriset( year, month, day, g_lon,g_lat, -35.0/60.0, 1,&sunRiseGMT, &sunSetGMT);

	//локализация времени: GMT + Часовой пояс
	g_sunRise= sunRiseGMT + g_sConfig.u8timeDiff;
	g_sunSet = sunSetGMT  + g_sConfig.u8timeDiff;
	if(g_sunRise>=24.0) g_sunRise-=24.0;
	if(g_sunSet>=24.0) g_sunSet-=24.0;

	//Канал 1
	//Время выключения освещения с учетом опережения (может быть отрицательным, если предыдущие сутки)
	g_RiseTime[1] = g_sunRise -  g_sConfig.ligthtChannels1.u8beforeSunRise/60.0;
	//Время включения освещения с учетом задержки (может быть больше или равно 24, если последующие сутки)
	g_SetTime[1] = g_sunSet + g_sConfig.ligthtChannels1.u8afterSunSet/60.0;

	//Канал 2
	g_RiseTime[2] = g_sunRise -  g_sConfig.ligthtChannels2.u8beforeSunRise/60.0;
	g_SetTime[2] = g_sunSet + g_sConfig.ligthtChannels2.u8afterSunSet/60.0;
	//Канал 3
	g_RiseTime[3] = g_sunRise -  g_sConfig.ligthtChannels3.u8beforeSunRise/60.0;
	g_SetTime[3] = g_sunSet + g_sConfig.ligthtChannels3.u8afterSunSet/60.0;
	//Канал 4
	g_RiseTime[4] = g_sunRise -  g_sConfig.ligthtChannels4.u8beforeSunRise/60.0;
	g_SetTime[4] = g_sunSet + g_sConfig.ligthtChannels4.u8afterSunSet/60.0;

}

uint8_t set_by_astr_time(u32 time,uint8_t Channel)
{

	// Астрономический таймер ( sunriset) выдает время восхода и заката на сутки
	// Алгоритм определения в каком состояние  д.б. уличное освещение (вкл или выкл):
	// 1.Время восхода (формат ЧЧ,ММ) минус минуты раннего выключения --> "восход-"  переменная g_RiseTime
	// 2.Если меньше нуля не страшно  (это предыдущий день)
	// 3.Время заката  (формат ЧЧ,ММ) плюс минуты позднего выключения --> "закат+"  переменная g_SetTime
	// 4.Если больше 24 не страшно  (это следующий день)
	// 5. Д.Б ВЫКЛючено, если текущее время находится  между "восход-" и "закат+"
	// 6. Иначе Д.Б ВКЛючено
    // Проверить sunriset при крайных условиях , нет ли 	"восход-" >= "закат+"? (никогда не выключится)


	long ltime = (long) time;
	struct tm* currenTime = gmtime(&ltime);
	float currTime =  (currenTime->tm_hour)*1.0 + (currenTime->tm_min)/60.0;

	uint8_t prev = g_stateLight;

	CLRBIT(g_errorCode,ERR_HANDMODE_TIMEOUT);

	if( (currTime<g_SetTime[Channel]) && (currTime>g_RiseTime[Channel]) )
	{
		return 0;
	}
	else
	{
		return 1;
	}

//	if(prev != g_stateLight) g_eventCode = (1<< g_stateLight);

}

//Мониторинг нештатных ситуации
void monitoring(u8 hdl, u32 time)
{

	long ltime = (long) time;
	struct tm* currenTime = gmtime(&ltime);
	int   year = currenTime->tm_year+1900;
	int   month = currenTime->tm_mon +1;
	int   day = currenTime->tm_mday;

	//контролируем аремя нахождения в ручном режиме
	//controlHandMode();

	//проверяем сброс времени
	if( year < 2019 )//TODO
	{
		SETBIT(g_errorCode,ERR_TIMER);

		if(g_sConfig.ligthtChannels1.u8ControlMode == CONTROLE_BY_ASTR_TIMER)
		{
			if(g_sConfig.ligthtChannels1.u8ControlMode != CONTROLE_BY_FHOTOSENSOR) //если уже не в режиме фотодатчика
			{
				if(g_sConfig.u8hardware & 0x01) //если работает фотодатчик,  то есть установлен и работоспособен
				{
					g_sConfig.ligthtChannels1.u8ControlMode = CONTROLE_BY_FHOTOSENSOR;
					toArchive(time,CONTROLE_BY_FHOTOSENSOR);
					g_eventCode = (1<< (CONTROLE_BY_FHOTOSENSOR + 8));
				}
				else
				{
					g_sConfig.ligthtChannels1.u8ControlMode = CONTROLE_BY_SERVER_HARD;
					//set_ligth(0);
				}
			}
		}
	}

	//проверка включенности днем и выключенности ночью освещения, и исправление данной ошибки
	uint8_t iChannel;
	for (iChannel= 1; iChannel <= 4; iChannel++)
	{
		if( (g_sControlMode[iChannel] == CONTROLE_BY_ASTR_TIMER) || (g_sControlMode[iChannel] == CONTROLE_BY_ASTR_SHEDULLE) )
		{
    		g_astrLight = set_by_astr_time(time,iChannel);
    		set_ligthByChannel(iChannel, g_astrLight);
		}
	}

	//*TODO для всех каналов
	//считываем сосотяния входного сигнала контактора и выходного сигнала контроллера
	if(GPIO_ReadInputDataBit(GPIO_PORT_CONTACTOR_IN, GPIO_PIN_CONTACTOR_IN) != GPIO_ReadOutputDataBit(GPIO_PORT_LIGHT, GPIO_PIN_LIGHT))
	{
		SETBIT(g_errorCode, ERR_OUT_MCU_AND_REAL);
		//TODO Попытаться восстановить ситуацию
	}
	else
	{
		CLRBIT(g_errorCode, ERR_OUT_MCU_AND_REAL);
	}


	//проверяем перезагрузку
	if(g_restart == 0)
	{
		SETBIT(g_errorCode,ERR_RESTART);
	}
	else
	{
		CLRBIT(g_errorCode,ERR_RESTART);
	}



}

void toArchive(u32 time,uint16_t event)
{
	if(!g_isArchive) return;
	if(iArchive >= ARCHIVE_SIZE) iArchive=0;
	g_event[iArchive].u32Ts = time;
	g_event[iArchive].u16eID = event;
	iArchive++;
}

//Проверка изменения состояния входа от фотодатчика
void photosensorCheck(u8 hdl, u32 time)
{

	g_timeout =  g_timeout + 5;
	if(g_timeout > g_sConfig.u16TimeOut)
	{
		if(g_sConfig.ligthtChannels1.u8ControlMode != CONTROLE_BY_SERVER)
		{
			g_sConfig.ligthtChannels1.u8ControlMode = CONTROLE_BY_ASTR_TIMER;
		}
		g_timeout = 0;
	}

	g_debug = 1-g_debug; if(g_debug){GPIO_SIG_LED_ON;}else{GPIO_SIG_LED_OFF;}

	uint8_t iChannel;
	for (iChannel= 1; iChannel <= 4; iChannel++)
	{
		if(g_sControlMode[iChannel] == CONTROLE_BY_FHOTOSENSOR)
		{
			g_state_photosensorIn= GPIO_ReadInputDataBit(GPIO_PORT_PHOTOSENSOR_IN, GPIO_PIN_PHOTOSENSOR_IN);
			//*TODO if(g_stateLight != g_state_photosensorIn)  g_eventCode = (1<< g_state_photosensorIn);
			set_ligthByChannel(iChannel, g_state_photosensorIn);
		}
	}

}

void error()
{
	//g_errorCode=0x1000;
	//Отправка при ошибке
	if( (g_countTry) && (g_errorCode > 0))    	//g_count_try - Количество попыток отправки одного и того же слова состояния (ошибок) ,
	{

		if(g_errorCode == errorCode)
		{
			if(g_timeErrorMessage != timeErrorMessage)
			{
				sendErrorTry();
			}
			else  //ошибка отправлена и принята
			{
				//g_errorCode = 0 ; нельзя обнулять (ошибки  от этого не исчезнут)
				tryes = g_countTry;
			}
		}
		else
		{
			tacts = 0;
			sendErrorTry();
		}
	}
	else
	{
		tryes=0;  //заткнется после g_countTryEvent = 3 попыток отправить одну и ту же ошибку
	}
}

void sendErrorTry()
{

	if((g_errorCode == errorCode))  //errorCode  присваивается значение g_errorCode после отправки ( в sendError)
	{
		//нет новых ошибок, а эта уже отправлена-> тогда:
		//пропускаем g_count_tacts кол-во попыток
		tacts++;
		if(tacts * MONITORING_TIME < g_errorSendInterval) return; //интервал между повторными отправками слова состояния ошибок), сек
	}
	else
	{
		tryes = 0;  // при изменение кода ошибок
	}

	if(tryes < g_countTry)
	{
		sendError();
		errorCode = g_errorCode;
		tacts = 0;
		tryes ++;
	}
	else
	{
	   CLRBIT(g_errorCode,ERR_RESTART);
	   g_restart = 1;
	}
}

void sendError()
{
	uint8_t ret[36];

	for(int i = 0;i < 16; i++)
	{
		ret[i] = g_objectId[i];
	}

    ret[16] = g_sConfig.u8NetworkAddress;
    ret[17] = 0;
    ret[18] = (uint8_t)(g_errorCode >> 8);
    ret[19] = (uint8_t)(g_errorCode & 0xFF);

    timeErrorMessage = RTC_GetCounter();
	uint8_t bytes[4];
	memcpy(bytes,&timeErrorMessage,4);
	ret[20] = bytes[0];
	ret[21] = bytes[1];
	ret[22] = bytes[2];
	ret[23] = bytes[3];

	ret[24] = g_stateLight;
	ret[25] = GPIO_ReadInputDataBit(GPIO_PORT_CONTACTOR_IN, GPIO_PIN_CONTACTOR_IN);
	ret[26] = GPIO_ReadInputDataBit(GPIO_PORT_PHOTOSENSOR_IN, GPIO_PIN_PHOTOSENSOR_IN);
	ret[27] = g_sConfig.ligthtChannels1.u8ControlMode;

    fullNA = g_fullNA;  g_fullNA = 1;
    FrameModbus_Send(0x4D, ret,28);
    g_fullNA = fullNA;
    g_restart = 1;
}

void event()
{
	//Отправка событий
	if( (g_countTryEvent) && (g_eventCode > 0))
	{
		if((g_eventCode == eventCode))  //повторная посылка событий
		{
			if(g_timeEventMessage != timeEventMessage)
			{
				sendEventTry();
			}
			else  //событие отправлено и принято
			{
				g_eventCode = 0 ;
				tactsEvent = 0;
			}
		}
		else
		{
			sendEventTry();
		}
	}
	else
	{
		tryesEvent=0;  //заткнется после g_countTryEvent = 3 попыток отправить одну и ту же ситуацию
	}
}


void sendEventTry()
{

	if((g_eventCode == eventCode))  //errorCode  присваивается значение g_errorCode после отправки ( в sendError)
	{
		//нет новых ошибок, а эта уже отправлена-> тогда:
		//пропускаем g_count_tacts кол-во попыток
		tactsEvent++;
		if(tactsEvent * MONITORING_TIME < g_errorSendInterval) return; //интервал между повторными отправками слова состояния ошибок), сек
	}
	else
	{
		tryesEvent = 0;  // при изменение кода ошибок
	}

	if(tryesEvent < g_countTry)
	{
		sendEvent();
		eventCode = g_eventCode;
		tactsEvent = 0;
		tryesEvent ++;
	}
}

void sendEvent()  //старший байт - переключение режимов управления; младший - вкл/выкл света ит.д
{
	//if(USART_GetITStatus(USART1, USART_IT_IDLE))  //состояние покоя
	{
	 	uint8_t ret[36];

		for(int i = 0;i < 16; i++)
		{
			ret[i] = g_objectId[i];
		}

	    ret[16] = g_sConfig.u8NetworkAddress;
	    ret[17] = 1; //event

	    ret[18] = (uint8_t)(g_eventCode >> 8);
	    ret[19] = (uint8_t)(g_eventCode & 0xFF);

	    timeEventMessage = RTC_GetCounter();
		uint8_t bytes[4];
		memcpy(bytes,&timeEventMessage,4);
		ret[20] = bytes[0];
		ret[21] = bytes[1];
		ret[22] = bytes[2];
		ret[23] = bytes[3];

        if(g_stateLight != GPIO_ReadInputDataBit(GPIO_PORT_CONTACTOR_IN, GPIO_PIN_CONTACTOR_IN))
        {
        	for(int jjjj=1000000;jjjj>0;jjjj--);
        }
		ret[24] = g_stateLight;
		ret[25] = GPIO_ReadInputDataBit(GPIO_PORT_CONTACTOR_IN, GPIO_PIN_CONTACTOR_IN);
		ret[26] = GPIO_ReadInputDataBit(GPIO_PORT_PHOTOSENSOR_IN, GPIO_PIN_PHOTOSENSOR_IN);
		ret[27] = g_sConfig.ligthtChannels1.u8ControlMode;

	    fullNA = g_fullNA;  g_fullNA = 1;
	    FrameModbus_Send(0x4D, ret,28);
	    g_fullNA = fullNA;
	}
}


void controlHandMode()
{
	//Канал 1
	//Установка флага и времени перехода в ручной режим
	if ((g_sConfig.ligthtChannels1.u8ControlMode & CONTROLE_BY_SERVER) == 0x0000) // не находится в режиме ручного управления,
	{
		HandModeStart1= time;										// запоминаем время последнего соcтояния не в ручном режиме
		CLRBIT(g_errorCode,ERR_HANDMODE_TIMEOUT);
	}
	else
	{
		//Проверка времени, прошедшего с начала перехода в ручной режим. g_time_manual- максимальное время нахождения в ручном времени
		if((time - HandModeStart1 ) > g_MaxTimeManual *60 )
		{
			if( g_sConfig.ligthtChannels1.u8ControlMode == CONTROLE_BY_SERVER)
			{
				g_sConfig.ligthtChannels1.u8ControlMode = CONTROLE_BY_ASTR_TIMER;
				toArchive(time,CONTROLE_BY_ASTR_TIMER);
				g_eventCode = (1 << CONTROLE_BY_ASTR_TIMER);
			}
			else
			{
				SETBIT(g_errorCode, ERR_HANDMODE_TIMEOUT);
			}
		}
	}

	//Канал 2
	//Установка флага и времени перехода в ручной режим
	if ((g_sConfig.ligthtChannels2.u8ControlMode & CONTROLE_BY_SERVER) == 0x0000) // не находится в режиме ручного управления,
	{
		HandModeStart2 = time;										// запоминаем время последнего соcтояния не в ручном режиме
		CLRBIT(g_errorCode,ERR_HANDMODE_TIMEOUT);
	}
	else
	{
		//Проверка времени, прошедшего с начала перехода в ручной режим. g_time_manual- максимальное время нахождения в ручном времени
		if((time - HandModeStart2 ) > g_MaxTimeManual *60 )
		{
			if( g_sConfig.ligthtChannels2.u8ControlMode == CONTROLE_BY_SERVER)
			{
				g_sConfig.ligthtChannels2.u8ControlMode = CONTROLE_BY_ASTR_TIMER;
				toArchive(time,CONTROLE_BY_ASTR_TIMER);
				g_eventCode = (1 << CONTROLE_BY_ASTR_TIMER);
			}
			else
			{
				SETBIT(g_errorCode, ERR_HANDMODE_TIMEOUT);
			}
		}
	}

	//Канал 3
	//Установка флага и времени перехода в ручной режим
	if ((g_sConfig.ligthtChannels3.u8ControlMode & CONTROLE_BY_SERVER) == 0x0000) // не находится в режиме ручного управления,
	{
		HandModeStart1 = time;										// запоминаем время последнего соcтояния не в ручном режиме
		CLRBIT(g_errorCode,ERR_HANDMODE_TIMEOUT);
	}
	else
	{
		//Проверка времени, прошедшего с начала перехода в ручной режим. g_time_manual- максимальное время нахождения в ручном времени
		if((time - HandModeStart1 ) > g_MaxTimeManual *60 )
		{
			if( g_sConfig.ligthtChannels3.u8ControlMode == CONTROLE_BY_SERVER)
			{
				g_sConfig.ligthtChannels3.u8ControlMode = CONTROLE_BY_ASTR_TIMER;
				toArchive(time,CONTROLE_BY_ASTR_TIMER);
				g_eventCode = (1 << CONTROLE_BY_ASTR_TIMER);
			}
			else
			{
				SETBIT(g_errorCode, ERR_HANDMODE_TIMEOUT);
			}
		}
	}

	//Канал 4
	//Установка флага и времени перехода в ручной режим
	if ((g_sConfig.ligthtChannels4.u8ControlMode & CONTROLE_BY_SERVER) == 0x0000) // не находится в режиме ручного управления,
	{
		HandModeStart4 = time;										// запоминаем время последнего соcтояния не в ручном режиме
		CLRBIT(g_errorCode,ERR_HANDMODE_TIMEOUT);
	}
	else
	{
		//Проверка времени, прошедшего с начала перехода в ручной режим. g_time_manual- максимальное время нахождения в ручном времени
		if((time - HandModeStart4 ) > g_MaxTimeManual *60 )
		{
			if( g_sConfig.ligthtChannels4.u8ControlMode == CONTROLE_BY_SERVER)
			{
				g_sConfig.ligthtChannels4.u8ControlMode = CONTROLE_BY_ASTR_TIMER;
				toArchive(time,CONTROLE_BY_ASTR_TIMER);
				g_eventCode = (1 << CONTROLE_BY_ASTR_TIMER);
			}
			else
			{
				SETBIT(g_errorCode, ERR_HANDMODE_TIMEOUT);
			}
		}
	}
}
#endif


