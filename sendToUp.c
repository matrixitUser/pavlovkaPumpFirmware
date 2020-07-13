#include "main.h"
#include "sendToUp.h"
static uint8_t tryes = 0;  //реальное количество отправок сообщения (попыток, если контролируем состояние порта)
static uint8_t tryesEvent = 0;  //реальное количество отправок сообщения (попыток, если контролируем состояние порта)
//PUBLIC uint32_t g_Event_timeMessageSend;
//PUBLIC uint32_t g_Event_timeMessageReceive;
//PUBLIC uint32_t g_Error_timeMessageSend;
//PUBLIC uint32_t g_Error_timeMessageReceive;
static uint16_t errorCode = 0;
static uint16_t eventCode = 0;
static uint8_t tacts = 0;
static uint8_t tactsEvent = 0;
static uint8_t fullNA;




void error(u32 time)
{
	//-printf("g_errorCode =%d; errorCode=%d\n\r",g_errorCode,errorCode);
	//g_errorCode=0x1000;
	//Отправка при ошибке
	if( (g_countTry) && (g_errorCode > 0))    	//g_count_try - Количество попыток отправки одного и того же слова состояния (ошибок) ,
	{

		if(g_errorCode == errorCode)
		{
			//-printf(" g_Error_timeMessageSend=%d; g_Error_timeMessageReceive=%d;\n\r",g_Error_timeMessageSend,g_Error_timeMessageReceive);
			if(g_Error_timeMessageSend != g_Error_timeMessageReceive)
			{
				sendErrorTry();
			}
			else  //ошибка отправлена и принята
			{
				//g_errorCode = 0 ; нельзя обнулять (ошибки  от этого не исчезнут)
				//-printf("  !!! g_Event_timeMessageSend == g_Event_timeMessageReceive=%d;\n\r",g_Event_timeMessageSend != g_Event_timeMessageReceive);
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
		//-printf(" 1 call sendMesagge:g_errorCode=%d \n\r",g_errorCode);
		sendMesagge(0, g_errorCode);
		//-printf(" 2 call sendMesagge:g_errorCode=%d \n\r",g_errorCode);
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

/////////////////////////// EVENT ///////////////////////////
void event(u32 time)
{
	//Отправка событий
	if( (g_countTryEvent) && (g_eventCode > 0))
	{
		if((g_eventCode == eventCode))  //повторная посылка событий
		{
			if(g_Event_timeMessageSend != g_Event_timeMessageReceive)
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


void sendEventTry(void)
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
		sendMesagge(1,g_eventCode);
		eventCode = g_eventCode;
		tactsEvent = 0;
		tryesEvent ++;
	}
}

void sendMesagge(uint8_t errorOrEvent, uint16_t message)     //старший байт - переключение режимов управления; младший - вкл/выкл света ит.д
{
	//-printf(" === sendMesagge  ==  message=%d,errorOrEvent=%x\n\r",message,errorOrEvent);
	uint8_t ret[100];
 	//----------------------- ШАПКА сообщения на верх  BEGIN  ------------------//
 	//Общие для всех: Object_id-16,байт, NA=1-байт,event Or error-1,g_eventCode-2,time-4
	for(int i = 0;i < 16; i++)    //14 ,FQ
	{
		ret[i] = g_objectId[i];
	}

    ret[16] = g_sConfig.u8NetworkAddress;     //30 байт
    ret[17] = errorOrEvent; //0-error;1 -event

    ret[18] = (uint8_t)(message >> 8);    //32
    ret[19] = (uint8_t)(message & 0xFF);
    u32 time = RTC_GetCounter();
    if (errorOrEvent)  g_Event_timeMessageSend = time;else   g_Error_timeMessageSend = time;  //

	uint8_t bytes[4];
	memcpy(bytes,&time,4);
	ret[20] = bytes[0];  //34
	ret[21] = bytes[1];
	ret[22] = bytes[2];
	ret[23] = bytes[3];
	//-printf(" sendMesagge:  ret[20-23]=%d,%d,%d,%d,\n\r",ret[20],ret[21],ret[22],ret[23] );
 	//----------------------- ШАПКА сообщения на верх  END  ------------------//

	uint8_t len=24;
	len = fillMessage(&ret,len);

    fullNA = g_fullNA;  g_fullNA = 1;
    FrameModbus_Send(MODBUS_USER_FUNCTION_ACK, ret,len);
    g_fullNA = fullNA;
	//-printf(" ==sendMesagge== time=%d\n\r",time);
}


uint8_t fillMessage(uint8_t *ret,uint8_t len)
{
	uint8_t bytes[4];
	//Сетевые адреса активных устройств
	if(g_leftUppActivity) ret[len] = UPP_LEFT.NA;
	if(g_rightUppActivity) ret[len] = UPP_RIGHT.NA;
	//////////////////////////////////////////Контроллер
	ret[len+2] = (
			(GPIO_ReadInputDataBit(GPIO_PORT_LEFT_STATUS_WORK, GPIO_PIN_LEFT_STATUS_WORK)<<1)
			| (GPIO_ReadInputDataBit(GPIO_PORT_RIGHT_STATUS_WORK, GPIO_PIN_RIGHT_STATUS_WORK)<<0)
			); //IN status
	ret[len+3] = (
			(GPIO_ReadOutputDataBit(GPIO_PORT_STOP, GPIO_PIN_STOP)<<3)
			| (GPIO_ReadOutputDataBit(GPIO_PORT_START, GPIO_PIN_START)<<3)
			//| (GPIO_ReadOutputDataBit(GPIO_PORT_STOP, GPIO_PIN_STOP)<<3)
			//| (GPIO_ReadOutputDataBit(GPIO_PORT_START, GPIO_PIN_START)<<3)
			); //OUT status
	//ошибки контроллера
	ret[len+4] = g_errorCode>>8;
	ret[len+5] = g_errorCode&0xFF;
	//события контроллера
	ret[len+6] = g_eventCode>>8;
	ret[len+7] = g_eventCode&0xFF;

	ret[len+8] = gTransferAllow; // Разрешение на трансфер

	ret[len+9] = g_sConfig.u16FlashVer>>8;
	ret[len+10] = g_sConfig.u16FlashVer & 0xFF;

	uint8_t leftUpp = g_leftUppActivity;
	uint8_t rightUpp = g_rightUppActivity;
	ret[len+11] = leftUpp<<1 | rightUpp<<0; // Состояния переключателя выбора УПП

	/////////////////////////////////////УПП

	//Время последнего опроса УПП
	if(g_leftUppActivity)
	{
		memcpy(bytes,&UPP_LEFT.lastRespTime,4);
		ret[len+12] = bytes[0];
		ret[len+13] = bytes[1];
		ret[len+14] = bytes[2];
		ret[len+15] = bytes[3];
	}
	if(g_rightUppActivity)
	{
		memcpy(bytes,&UPP_RIGHT.lastRespTime,4);
		ret[len+16] = bytes[0];
		ret[len+17] = bytes[1];
		ret[len+18] = bytes[2];
		ret[len+19] = bytes[3];
	}

	//Байты состояния УПП
	if(g_leftUppActivity)
	{
		ret[len+20] = UPP_LEFT.status>>8;
		ret[len+21] = UPP_LEFT.status&0xFF;
	}
	if(g_rightUppActivity)
	{
		ret[len+20] = UPP_RIGHT.status>>8;
		ret[len+21] = UPP_RIGHT.status&0xFF;
	}

	//////////////////////////////////WLS

	//Время последнего опроса WLS
	memcpy(bytes,&g_WLS.lastRespTime,4);
	ret[len+22] = bytes[0];
	ret[len+23] = bytes[1];
	ret[len+24] = bytes[2];
	ret[len+25] = bytes[3];

	//Байт состояния WLS
	ret[len+26] = g_WLS.status>>8;
	ret[len+27] = g_WLS.status&0xFF;

	//Датчик давления
	//Время последнего опроса Датчика давления reserved
	memcpy(bytes,&g_WLS.lastRespTime,4);
	ret[len+28] = 0xFF;
	ret[len+29] = 0xFF;
	ret[len+30] = 0xFF;
	ret[len+31] = 0xFF;

	//Байт состояния Датчика давления reserved
	ret[len+32] = 0xFF;
	ret[len+33] = 0xFF;
    return len+34;
}


