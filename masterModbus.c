
#include "masterModbus.h"
#include "answerFrameModbus.h"
#include "stm32f10x_tim.h"
#ifdef PAVLOVKA

uint8_t requestIdle=1;
static uint8_t turn=0;
uint8_t K1countOn = 0;
uint8_t isK1on = 0;



void masterGpioInit()
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = GPIO_PIN_STOP;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;//GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = GPIO_PIN_START|GPIO_PIN_STOP;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD; //GPIO_Mode_Out_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);


	GPIO_ResetBits(GPIO_PORT_STOP, GPIO_PIN_STOP);
	GPIO_ResetBits(GPIO_PORT_START, GPIO_PIN_START);
	GPIO_ResetBits(GPIO_PORT_STOP, GPIO_PIN_STOP);
	GPIO_ResetBits(GPIO_PORT_START, GPIO_PIN_START);

    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Pin   = GPIO_PIN_STOP;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = GPIO_PIN_START|GPIO_PIN_STOP;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP; //GPIO_Mode_Out_OD;
    GPIO_Init(GPIOB, &GPIO_InitStructure);


    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Pin   = GPIO_PIN_RESTART_OTHER_MC;
	GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;//GPIO_Mode_Out_PP;
	GPIO_Init(GPIO_PORT_RESTART_OTHER_MC, &GPIO_InitStructure);
}

void master_init(void)
{
	masterGpioInit();
	TIM2_Init();
	TIM_Cmd(TIM2, DISABLE);
	TIM3_Init();
	TIM_Cmd(TIM3, DISABLE);
	TIM4_Init();
	TIM_Cmd(TIM4, DISABLE);
	TIM7_Init();
	TIM_Cmd(TIM7, DISABLE);
	//u8 crc8 = CalcCRC8FromChId();
	//for(uint32_t ii = 0; ii < crc8*100000; ii++);
	//printf("crc8 = %d \n\r", crc8);

	//printf("---first.start=%d,first.stop=%d,second.start=%d,second.stop=%d\n\r",g_UPPstruct->lastStartTime, g_UPPstruct->lastStopTime, g_SecondUPP->lastStartTime, g_SecondUPP->lastStopTime);
	ReadFromBKP();
	printf("+++first.start=%d,first.stop=%d,second.start=%d,second.stop=%d\n\r",g_UPPstruct->lastStartTime, g_UPPstruct->lastStopTime, g_SecondUPP->lastStartTime, g_SecondUPP->lastStopTime);
	GPIO_ResetBits(GPIOA, GPIO_Pin_6); // Отключение LED рестарта
//GPIO_SetBits(GPIO_PORT_RESTART_OTHER_MC, GPIO_PIN_RESTART_OTHER_MC);
//DELAY50000;
//GPIO_ResetBits(GPIO_PORT_RESTART_OTHER_MC, GPIO_PIN_RESTART_OTHER_MC);


	equipment_config();
	contactorSetOn();
	//SWAlarm_AddInterval(gTimerForTalking, gTimerForTalking, TalkingBtwnCtrs);
	SWAlarm_AddInterval(MONITORING_TIME, 2,  masterRequest);
	SWAlarm_AddInterval(3, 1,  led5blink);
	SWAlarm_AddInterval(32, 10, reqUPPTo_CONTROL_BY_SERVER);

}


void masterRequest(u8 hdl, u32 time)
{
	//controlUnitRqst();  //опрос контроллера управления сбросом
	//if(!UPPchoice()) return;
	UPPchoice();
	if(gControlMode == CONTROL_BY_AUTO)
	{
		gTransferAllow = 0;  //?
//		monitoring();
		tsCallback askingWlsCb;
		askingWlsCb.cb = monitoring;
		Callback_Save(&askingWlsCb);

		gTransferAllow = 1;  //?
	}

	//***turn=1-turn; if(turn)  event(time); else error(time);  //в разных тактах вызывается посылка ошибок и событий
}

void monitoring(u8 hdl, u32 param)
{
	if(!requestIdle) return; //Пропускаем, если пред.цикл опроса не закончен
	requestIdle = 0;
	TalkingBtwnCtrs();

	if(g_sConfig.u8Mode == MODE_SLAVE && (!gIsRestartMC || gCountNotAnswerController < 2))
	{
		printf("g_sConfig.u8Mode == MODE_SLAVE && (!gIsRestartMC || gCountNotAnswerController < 2) \n\r");
		return;
	}
	//printf("+++++++++++++++++monitoring+++++++++++++++++++++++ \n\r");
	if(!CheckConfigNew(&g_sConfig)) ReadConfig();;
	if(!CheckConfig(&g_sConfig)) ReadConfig();
	uint8_t count;


	//UPPstatusRqst();
	/*//20200629 Ильмир
	if(g_UPPstruct->engineSts == ENGINE_START_STATUS)
	{
		contactorSetOffTry();
	}
	 */
	uint8_t wlsOk = WlsStatusRqst();  //внутри  3 попытки опросить башню
	wait_(1);


	//printf("monit g_UPPstruct->na=%d \n\r",g_UPPstruct->NA);
	UPPstatusRqst(g_UPPstruct); //UPPstatusRqst();
	//printf("monit wlsOk=%d isUsedSecondUpp=%d \n\r",wlsOk, g_sConfig.isUsedSecondUpp);
	if(g_sConfig.isUsedSecondUpp == SECOND_UPP_IS_USED) UPPstatusRqst(g_SecondUPP);
 	//Опрос Башни (если хотя бы три секунды в одном и том же состоянии (максимум или минимум)
	if( wlsOk ) //опросилась Башня напрямую или получил данные из сервера
	{
		//printf("if( wlsOk ) \n\r");
		//g_WLS.status = 0;
		//printf("g_UPPstruct->engineSts = %d \n\r", g_UPPstruct->engineSts);
		if(g_WLS.status <= 1 && g_UPPstruct->engineSts != ENGINE_START_STATUS)
		{
			printf("MINIMUM <= 1  && STOP  g_UPPstruct=%d\n\r ", g_UPPstruct->engineSts);
			UPPstartTry(g_UPPstruct); //UPPstartTry();
			return;
		}
		else if(g_WLS.status == 0  && g_sConfig.isUsedSecondUpp == SECOND_UPP_IS_USED && isMinPumpTimeAfterStartUpp() && g_SecondUPP->engineSts != ENGINE_START_STATUS)//
		{
			printf("MINIMUM = 0  && START g_UPPstruct=%d  g_SecondUPP=%d\n\r ", g_UPPstruct->engineSts, g_SecondUPP->engineSts);
			UPPstartTry(g_SecondUPP);
			return;
		}
		else if(g_WLS.status >= 4)
		{
			if(g_UPPstruct->engineSts == ENGINE_START_STATUS)
			{
				printf("MАXIMUM  && START g_UPPstruct=%d\n\r", g_UPPstruct->engineSts);
				UPPstop(g_UPPstruct); //UPPstop();
			}

			if(g_sConfig.isUsedSecondUpp == SECOND_UPP_IS_USED && g_SecondUPP->engineSts == ENGINE_START_STATUS )
			{
				printf("MАXIMUM  && START g_SecondUPP=%d\n\r", g_SecondUPP->engineSts);
				UPPstop(g_SecondUPP);
			}
		}
		else
		{
			printf("g_WLS.status = %d g_UPPstruct->engineSts=%d g_SecondUPP->engineSts=%d\n\r",g_WLS.status, g_UPPstruct->engineSts, g_SecondUPP->engineSts);
		}
	}

	//Не простивает ли башня?
	if(g_UPPstruct->engineSts == ENGINE_STOP_STATUS)
	{
		//printf("wlsOk=%d isMaxPumpIdleTime(wlsOk)=%d wlsOk=%d\n\r",wlsOk, isMaxPumpIdleTime(wlsOk), wlsOk);
		if(isMaxPumpIdleTime(wlsOk) && !wlsOk) //Запуск насоса по причине не опроса Башни и превышение времени простоя
		{
			//printf("if(isMaxPumpIdleTime(wlsOk) && !wlsOk)\n\r");
			UPPstartTry(g_UPPstruct);
			return;
		}
	}

	printf("end monitoring\n\r");
	requestIdle = 1;
}
//общение между контроллерами
void TalkingBtwnCtrs(){
	if(g_sConfig.u8Mode == MODE_MASTER)
	{
		uint8_t ret1[44];
		CMDForTalkingBetweenControllers(&ret1);
		TalkingBtwnCtrls_Send(TALKING_BTWN_CONTROLLERS, MODBUS_CMD_SEND, ret1, sizeof(ret1));
	}
	if(gCountNotAnswerController < 3) gCountNotAnswerController++;
	//printf("gCountNotAnswerController = %d \n\r", gCountNotAnswerController);
	if(gCountNotAnswerController > 1)
	{
		printf("gCountNotAnswerController = %d  > 1\n\r", gCountNotAnswerController);
		if(!gIsRestartMC)
		{
			printf("RestartMC\n\r");
			//GPIO_SetBits(GPIO_PORT_RESTART_OTHER_MC, GPIO_PIN_RESTART_OTHER_MC);
			gCountNotAnswerController = 0;
			//DELAY50000;
			//GPIO_ResetBits(GPIO_PORT_RESTART_OTHER_MC, GPIO_PIN_RESTART_OTHER_MC);
		}
		gIsRestartMC = 1;
	}
}

void UPPstartTry(softStarter* Upp)  //Три попытки запустить насос
{
	UPPstatusRqst(Upp);
	if(Upp->engineSts == ENGINE_START_STATUS)
	{
		requestIdle = 1;
		return;
	}
	uint8_t tryAutoModeSetOn = 3;
	uint8_t autoModeSetOn = 0;
	uint8_t isUPPstart = 0;
	printf("UPPstartTry Upp->NA=%d \n\r",Upp->NA);

	if(!UPPfaultReset(Upp))
	{
		Call_TIM7(UPPstartCallback, Upp->NA);
		return; //попытка спроса ошибки
	}

	while( !autoModeSetOn  && tryAutoModeSetOn-- )
	{
		autoModeSetOn = UPPautoModeSetOn(Upp);
		wait_(1);
	}

	if(!autoModeSetOn)//принудительный способ перевода в автомод
	{
		printf("if(!autoModeSetOn) \n\r");
		Call_TIM7(UPPstartCallback, Upp->NA);
		return;
	}

	if(UPPstartRqst(Upp))//(UPPstartRqst())
	{
		printf("UPP START SUCCESS\n\r");
		Upp->lastStartTime = RTC_GetCounter();
		WriteToBKPLastStartTime(Upp);
		g_eventCode = UPP_START;
		requestIdle = 1;
	}
	else
	{
		printf("UPP START ERROR\n\r");
		g_errorCode = ERR_UPP_NO_START;
		Call_TIM7(UPPstartCallback, Upp->NA);
		return;
	}
}
void UPPstartCallback(u8 hdl, u32 param)
{
	static u8 triesUppStart = 3;
	softStarter* Upp;
	if(hdl == g_UPPstruct->NA) Upp = g_UPPstruct;
	else Upp = g_SecondUPP;
	printf("UPPstartCallback Upp->NA=%d hdl=%d\n\r",Upp->NA, hdl);
	uint8_t tryAutoModeSetOn = 3;
	uint8_t autoModeSetOn = 0;
	UPPstatusRqst(Upp);
	if(Upp->engineSts  == ENGINE_START_STATUS)
	{
		triesUppStart = 3;
		requestIdle = 1;
		return;
	}
	wait_(1);

	if(!UPPfaultReset(Upp))
	{
		printf("UPPstartCallback if(!UPPfaultReset(Upp)) triesUppStart=%d\n\r",triesUppStart);
		if(triesUppStart)
		{
			triesUppStart--;
			Call_TIM7(UPPstartCallback, Upp->NA);//65sec
		}
		else
		{
			triesUppStart = 3;
			requestIdle = 1;
		}
		return;
	}

	while( !autoModeSetOn  && tryAutoModeSetOn-- )
	{
		autoModeSetOn = UPPautoModeSetOn(Upp);
		wait_(1);
	}

	/*
	if(!autoModeSetOn)
	{
		printf("UPPstartCallback if(!autoModeSetOn) triesUppStart=%d\n\r",triesUppStart);
		if(triesUppStart)
		{
			triesUppStart--;
			Call_TIM7(UPPstartCallback, Upp->NA);//65sec
		}
		else
		{
			triesUppStart = 3;
			requestIdle = 1;
		}
		return;
	}
*/
	if(UPPstartRqst(Upp))
	{
		printf("UPP START SUCCESS\n\r");
		triesUppStart = 3;
		Upp->lastStartTime = RTC_GetCounter();
		WriteToBKPLastStartTime(Upp);
		g_eventCode = UPP_START;
	}
	else
	{
		printf("UPPstartCallback UPP START ERROR triesUppStart=%d\n\r",triesUppStart);
		g_errorCode = ERR_UPP_NO_START;
		if(triesUppStart)
		{
			triesUppStart--;
			Call_TIM7(UPPstartCallback, Upp->NA);//65sec
		}
		else
		{
			triesUppStart = 3;
			requestIdle = 1;
		}
		return;
	}
	printf("end UPPstartCallback Upp->NA=%d hdl=%d\n\r",Upp->NA, hdl);
	requestIdle = 1;
}

void Call_TIM7(ACTION_DATA(Cb), u8 hdl)
{
	TIM_Cmd(TIM7,ENABLE); // 65 sec
	g_tsCb.cb = Cb;
	g_tsCb.hdl = hdl;
}
void SaveToUpp(softStarter* Upp){
	if(Upp->NA == g_UPPstruct->NA) g_UPPstruct = Upp;
	else g_SecondUPP = Upp;
}


uint8_t UPPstatusRqst(softStarter* Upp)   //Запрос статуса UPP
{
	//printf("UPPstatusRqstNew Upp->na=%d \n\r",Upp->NA);
 	uint8_t data[4]={0x00, 0x00, 0x00, 0x10};
 	uint8_t fun = 0x01;
 	int8_t lengthWait =-1;
 	if(MasterModbusSend(Upp->NA, fun, data, sizeof(data), lengthWait, 3, 1))
 	{
 		//printf("%10d:Upp->engineSts=%x\n\r ", RTC_GetCounter(),	Upp->engineSts);
 		UPPparse(&g_frame, Upp);
 		return 1;
 	}
 	return 0;
}

uint8_t UPPstop(softStarter* Upp)
{
	_printf(2, "UPPstop()\n\r");
	uint8_t tryAutoModeSetOn = 3;
	uint8_t autoModeSetOn = 0;
	uint8_t isUPPstop=0;
	wait_(1);
	if(!UPPfaultReset(Upp)) return 0;

	while( !autoModeSetOn  && tryAutoModeSetOn-- )
	{
		autoModeSetOn = UPPautoModeSetOn(Upp);//он может из-за него запускать  , я думаю из-за того что контактор остался включенным---
		wait_(1);
	}

	/*
	if(!autoModeSetOn)
	{
		wait_(65);
	}
	while( !autoModeSetOn  && tryAutoModeSetOn-- )
	tryAutoModeSetOn =3;
	{
		autoModeSetOn = UPPautoModeSetOn(Upp);
		wait_(1);
	}

	if(!autoModeSetOn)
	{
		g_errorCode = ERR_UPP_AUTOMODE_SET;
		return 0;
	}
*/

	isUPPstop = !UPPstopRqst(Upp); //разве не так?
	if(isUPPstop)   //1- остановился
	{
		Upp->lastStopTime = RTC_GetCounter();
		WriteToBKPLastStopTime(Upp);
		g_eventCode = UPP_STOP;
		uint8_t count =3;
		while(	(CHECK_BIT(Upp->status, MODBUS_REG_DI_START_STATUS)
				||
				CHECK_BIT(Upp->status, MODBUS_REG_DI_STOP_STATUS) )
				&&
				count--
			)
		{
			wait_(1);
			UPPstatusRqst(Upp);//зачем еще раз дела
		}
		if(	!CHECK_BIT(Upp->status, MODBUS_REG_DI_START_STATUS)
				&&
			!CHECK_BIT(Upp->status, MODBUS_REG_DI_STOP_STATUS) )
		{
			UPPautoModeSetOff(Upp);
		}
		_printf(2, "4)Upp->status = %x\n\r",Upp->status);
	}
	else
	{
		g_errorCode = ERR_UPP_NO_STOP;
	}

	return isUPPstop;
}



uint8_t UPPstatusRqstExt(softStarter* Upp)  //дополнительные параметры
{
	_printf(2, "UPPstatusRqstExtNew()\n\r");
 	uint8_t data[4]={0x02, 0x00, 0x00, 0x0D};
 	uint8_t fun = 0x04;
 	int8_t lengthWait =-1;
 	if(MasterModbusSend(Upp->NA, fun, data, sizeof(data), lengthWait, 3, 1))
 	{
 		UPPparseExt(&g_frame, Upp);
 		_printf(2, "%10d:Upp->engineSts=%x\n\r ", RTC_GetCounter(),	Upp->engineSts);
 		return 1;
 	}
 	return 0;
}

void UPPparseExt(uint8_t* data, softStarter* Upp)
{
	uint8_t* mfd = data+1;  //  старший байт запрашиваемого регистра
	Upp->motorCurrent  		= ((mfd[1] <<8)|(mfd[2]));
	Upp->thermalLoad   		= ((mfd[3] <<8)|(mfd[4]));
	Upp->currentPhase1 		= ((mfd[5] <<8)|(mfd[6]));
	Upp->currentPhase2 		= ((mfd[7] <<8)|(mfd[8]));
	Upp->currentPhase3 		= ((mfd[9]<<8)|(mfd[10]));
	Upp->currentPhaseMax 	= ((mfd[11]<<8)|(mfd[12]));
	Upp->frequency 			= ((mfd[13]<<8)|(mfd[14]));
	Upp->power 				= ((mfd[15]<<8)|(mfd[16]));
	Upp->voltage 			= ((mfd[17]<<8)|(mfd[18]));
	Upp->startCount 		= ((mfd[19]<<8)|(mfd[20]));
	Upp->runtimeHours 		= ((mfd[21]<<8)|(mfd[22]));
	Upp->modbusError 		= ((mfd[23]<<8)|(mfd[24]));
	Upp->modbusToglle 		= ((mfd[25]<<8)|(mfd[26]));
}

uint8_t UPPstartRqst(softStarter* Upp)   //так каким образом?
{
	uint8_t uppOnTry = 3;
 	uint8_t data[4] = {0x01, 0x02, 0xFF, 0x00};
 	uint8_t fun = 0x05;
 	int8_t lengthWait =-1;

 	UPPstatusRqst(Upp);
 	//if() return 0;
 	while ((Upp->engineSts == ENGINE_STOP_STATUS) && uppOnTry--)
 	{
	 	if(!MasterModbusSend(Upp->NA, fun, data, sizeof(data), lengthWait, 3, 1)) continue;
		wait_(5);
		if(!UPPstatusRqst(Upp)) continue;
 	}
	return Upp->engineSts;
}

uint8_t UPPstopRqst(softStarter* Upp)
{
	uint8_t uppOffTry=3;
	//$2F$05$01$01$FF$00$DA$48 - запрос на остановку двигателя
 	uint8_t data[4] = {0x01, 0x01, 0xFF, 0x00};
 	uint8_t fun = 0x05;
 	int8_t lengthWait =-1;
 	UPPstatusRqst(Upp);
 	//if() return 0;  //не опросилось
 	while ((Upp->engineSts == ENGINE_START_STATUS) && uppOffTry--)
 	{
	 	if(!MasterModbusSend(Upp->NA, fun, data, sizeof(data), lengthWait, 3, 1)) continue;
		wait_(5);
		if(!UPPstatusRqst(Upp)) continue;
 	}
	return Upp->engineSts; //если ноль, значит остановился
}

//сброс ошибки и проверка состояния, если ошибка осталась то 0
uint8_t UPPfaultReset(softStarter* Upp)
{
 	uint8_t data[4] = {0x01,0x06, 0xFF, 0x00};
 	uint8_t fun = 0x05;
 	int8_t lengthWait =-1;
 	if(MasterModbusSend(Upp->NA, fun, data, sizeof(data), lengthWait, 3, 1))
 	{
 		if(UPPstatusRqst(Upp))
 		{
 			if(!CHECK_BIT(Upp->status, MODBUS_REG_FAULT_STATUS_SOFTSTARTER)) return 1;
 		}
 	}
 	return 0;
}
//попытка перевести упп в режим mode on
uint8_t UPPautoModeSetOn(softStarter* Upp)
{
 	uint8_t data[4] = {0x01, 0x05, 0xFF, 0x00};
 	uint8_t fun = 0x05;
 	int8_t lengthWait =-1;
 	if(MasterModbusSend(Upp->NA, fun, data, sizeof(data), lengthWait, 3, 1))
 	{
 		if(UPPstatusRqst(Upp))
 		{
 			if(CHECK_BIT(Upp->status, MODBUS_REG_AUTOMODE_STATUS_SOFTSTARTER)){
 				SaveToUpp(Upp);
 				return 1;
 			}
 		}
 	}
 	return 0;
}

////////////////////////////////////PARSING'S/////////////////////////////////////////
uint8_t UPPautoModeSetOff(softStarter* Upp)
{
	uint8_t data[4] = {0x01, 0x05, 0x00, 0x00};
 	uint8_t fun = 0x05;
 	int8_t lengthWait =-1;
 	if(MasterModbusSend(Upp->NA, fun, data, sizeof(data), lengthWait, 3, 1))
 	{
 		if(UPPstatusRqst(Upp))
 		{
 			if(!CHECK_BIT(Upp->status, MODBUS_REG_AUTOMODE_STATUS_SOFTSTARTER)) return 1;
 		}
 	}
 	return 0;
}

uint8_t UPPautoModeSetOffByControlWord(softStarter* Upp)
{
 	uint8_t data[4] = {0x04, 0x05, 0x00, 0x00};
 	uint8_t fun = 0x05;
 	int8_t lengthWait =-1;
 	return MasterModbusSend(Upp->NA, fun, data, sizeof(data), lengthWait, 3, 1);
}



//////////////////////////////////WLS
uint8_t myWLSstatusRqst()
{
 	uint8_t data[4]={0x01, 0x00, 0x00, 0x01};
 	uint8_t fun = 0x04;
 	int8_t lengthWait =-1;

	//printf("g_sConfig.WlsServerMode = %d \n\r", g_sConfig.WlsServerMode);
 	if(g_sConfig.WlsServerMode == WLSMODE_SERVER_ONLY) //Использовать данные из сервера
 	{
 		g_frame[0] = fun;g_frame[1]=2;g_frame[2]=0;g_frame[3]=g_WLSfromserver;
		WLSparse(&g_frame); // а высота - сразу из сервера
		return 1;
 	}

	if(!MasterModbusSendUsart2(g_WLS.NA, fun, data, sizeof(data), lengthWait, 3, 1))  //нет ответа от Башни со 2 порта
	{
		if(g_sConfig.WlsServerMode == WLSMODE_MC_SERVER)  //10 04 02 00 0F crc  - данные по башне получаем из сервера
		{
			if(g_WLSfromserver != 0xFF)   //сервер отдал WLS
			{
				g_frame[0] = fun;g_frame[1]=2;g_frame[2]=0;g_frame[3]=g_WLSfromserver;  //g_WLSfromserver- Отдает сервер
				WLSparse(&g_frame); // а высота - сразу из сервера
				return 1;
			}
			else // Не было данных по Башне из сервера, необходимо смотреть время простоя насоса
			{
				return 0;
			}
		}
		else  //ни с порта 2, ни с контроллера(порт3)
		{
			g_errorCode = ERR_WLS_NOT_RESPONCE;
		}
		return 0;
	}
	//Башня нормально опрашивается, тогда
	WLSparse(&g_frame);
	g_WLSfromserver = 0xFF;
	g_WLS.height = 0;
    wait_(1);
 	uint8_t data1[4]={0x0B, 0x00, 0x00, 0x02};
	if(!MasterModbusSendUsart2(g_WLS.NA, fun, data1, sizeof(data1), lengthWait, 3, 1))
	{
		g_errorCode = ERR_WLS_NOT_RESPONCE;
		return 0;

	}
	g_WLS.height = 0;
	WLSparseHeight(&g_frame);

	_printf(2, "lastRespTime = %d, status = %d\n\r", g_WLS.lastRespTime, g_WLS.status);
	_printf(2, "height = %i.%02i meter\n\r", (int)(g_WLS.height),(int)(g_WLS.height*100) - ((int)g_WLS.height)*100);
	return 1;
}

uint8_t WlsStatusRqst(){
	uint8_t wlsOk = myWLSstatusRqst();
	printf(" wlsOk = %d  g_WLS.status = %d height=%d\n\r",wlsOk, g_WLS.status, (int)g_WLS.height*100);
	if(!wlsOk){
		//printf("111wlsOk=%d gIsRestartMC=%d\n\r",wlsOk, gIsRestartMC);
		if(gIsRestartMC) return 0;
		//printf("222wlsOk=%d gIsRestartMC=%d\n\r",wlsOk, gIsRestartMC);
		//попросить slave опросить wls
		uint8_t ret1[4];
		AckingWLSToMC(&ret1);
		if(!Talking_Send(TALKING_ASKING_WLS, ret1)) return 0;
		if(g_dataTalk[0] == 0)
		{
			//printf(" g_dataTalk[0] == 0\n\r");
			g_sConfig.WlsServerMode = WLSMODE_MC_SERVER;
			return 0;
		}
		uint8_t tmp[4];
		//wls
		tmp[0] = 0x04;
		tmp[1] = 2;
		tmp[2] = g_dataTalk[1];
		tmp[3] = g_dataTalk[2];
		WLSparse(&tmp); // а высота - сразу из сервера
		//height
		union
		{
			float float_variable;
			uint8_t inBytes[4];
		} uu;

		uu.inBytes[3] = g_dataTalk[6];
		uu.inBytes[2] = g_dataTalk[5];
		uu.inBytes[1] = g_dataTalk[4];
		uu.inBytes[0] = g_dataTalk[3];
		g_WLS.height = uu.float_variable;
		printf("g_WLS.status = %d height=%d\n\r",g_WLS.status, (int)g_WLS.height*100);
	}
	//printf("after if(!wlsOk && !gIsRestartMC)\n\r");
	return 1;
}


////////////////////////////Parsing/////////////////////////////////////
void UPPparse(uint8_t* data, softStarter* Upp)
{
	volatile uint8_t* mfd = data+1;  //  старший байт запрашиваемого регистра

	Upp->lastRespTime = RTC_GetCounter();
	Upp->oldStatus = Upp->status;
	Upp->status = ((mfd[2]<<8)|(mfd[1]));

	if(CHECK_BIT(Upp->status, MODBUS_REG_STOP_STATUS_SOFTSTARTER) && !CHECK_BIT(Upp->status, MODBUS_REG_START_STATUS_SOFTSTARTER))
	{
		//TODO УПП остановлено
		Upp->engineSts = ENGINE_STOP_STATUS;
		if( (Upp->lastStopTime == TIME_UNDEFINED) || (Upp->lastStopTime <= TIME_RELEASE))
		{
			ReadFromBKP();
			Upp->lastStopTime = RTC_GetCounter();
			WriteToBKPLastStopTime(Upp);
		}
		K1countOn = 0;
	}
	if(CHECK_BIT(Upp->status, MODBUS_REG_START_STATUS_SOFTSTARTER) && !CHECK_BIT(Upp->status, MODBUS_REG_STOP_STATUS_SOFTSTARTER))
	{
		//TODO УПП запущен
		Upp->engineSts = ENGINE_START_STATUS;
		if( (Upp->lastStartTime == TIME_UNDEFINED) || (Upp->lastStartTime <= TIME_RELEASE))
		{
			Upp->lastStartTime = RTC_GetCounter();
			WriteToBKPLastStartTime(Upp);
		}
 		K1countOn++; if(K1countOn>10) K1countOn=11;
	}
	SaveToUpp(Upp);
}
void WLSparse(uint8_t* data) //только команда опроса уровня...
{
	volatile uint8_t* mfd = data+1;  //  старший байт запрашиваемого регистра

	g_WLS.lastRespTime = RTC_GetCounter();
	g_WLS.oldStatus = g_WLS.status;
	g_WLS.status = ((mfd[1]<<8)|(mfd[2])); // 0000 0000| 0001 0011

	if(CHECK_BIT(g_WLS.status,MODBUS_REG_MIN_WATER_STATUS))
	{
		g_WLS.minLvl = 1;
	}
	else
	{
		g_WLS.minLvl = 0;
	}

	if(CHECK_BIT(g_WLS.status,MODBUS_REG_MAX_WATER_STATUS))
	{
		g_WLS.maxLvl = 1;
	}
	else
	{
		g_WLS.maxLvl = 0;
	}
}

void WLSparseHeight(uint8_t* data) //только команда опроса уровня...
{
	uint8_t* mfd = data+2;  //  старший байт запрашиваемого регистра
	union
	{
	    float float_variable;
	    uint8_t inBytes[4];
	} uu;

	uu.inBytes[2] = mfd[3];
	uu.inBytes[3] = mfd[2];
	uu.inBytes[0] = mfd[1];
	uu.inBytes[1] = mfd[0];
	g_WLS.height = uu.float_variable;
}

uint8_t isMaxPumpIdleTime(uint8_t wlsOk)
{
	if( (g_UPPstruct->lastStopTime != TIME_UNDEFINED) && (g_UPPstruct->lastStopTime > TIME_RELEASE)) //Время похожее на правду
	{
		//printf("g_sConfig.PumpTimeIdleMax = %d\n\r", g_sConfig.PumpTimeIdleMax*4);
		if( (RTC_GetCounter() - g_UPPstruct->lastStopTime) > g_sConfig.PumpTimeIdleMax*4)
		{
			//printf("if((RTC-lastStopTime)>PumpTimeIdleMax*4)\n\r");
			if(!wlsOk)  //насос стоит больше положенного и нет ответа от башни
			{
				g_errorCode = ERR_PUMP_IDLE_MAX;
			}
			else
			{
				g_eventCode = ERR_PUMP_IDLE_MAX;
			}
			return 1;
		}
	}
	return 0; //Максимум времени простоя не достигнут
}

uint8_t isMinPumpTimeAfterStartUpp()
{
	printf("g_UPPstruct->lastStartTime = %d, MinTimeAfterStart=%d \n\r", g_UPPstruct->lastStartTime, g_sConfig.MinTimeAfterStart);
	if(g_UPPstruct->engineSts != ENGINE_START_STATUS ) return 0;
	//if( (g_UPPstruct->lastStartTime != TIME_UNDEFINED) && (g_UPPstruct->lastStartTime > TIME_RELEASE)) //Время похожее на правду
	{
		if( (RTC_GetCounter() - g_UPPstruct->lastStartTime) > g_sConfig.MinTimeAfterStart)
		{
			return 1;
		}
	}
	return 0;
}
////////////////////////////////////END PARSING'S/////////////////////////////////////////

////////////////////////////////CONTACTOR////////////////////////////////////
//
uint8_t contactorSetOn()
{
//return 0;
	_printf(2, "contactorSetOn\n\r");
	GPIO_SetBits(GPIO_PORT_START, GPIO_PIN_START);//PB1
	wait_(2);
	GPIO_ResetBits(GPIO_PORT_START, GPIO_PIN_START);
	isK1on == 1;
	return contactorStatus();
}

uint8_t contactorSetOffTry()
{
	_printf(2, "contactorSetOffTry: ");
	UPPstatusRqsNew(g_UPPstruct); 	wait_(1); 	UPPstatusRqstExtNew(g_UPPstruct);
	_printf(2, "1)g_UPPstruct->status = %x\n\r",g_UPPstruct->status);
	if(
		!(
		CHECK_BIT(g_UPPstruct->status, MODBUS_REG_START_STATUS_SOFTSTARTER)
		&&
		CHECK_BIT(g_UPPstruct->status, MODBUS_REG_TOR_STATUS)
		&&
		(K1countOn == 10)  //10 раз должен находится в состоянии старт
		)
	  )
	{
		return !contactorStatus();
	}
	//отключение реле К1; Контактор удерживается реле K4;Реле К4-удерживается сигналом RUN от УПП
	return contactorSetOff();
}

uint8_t contactorSetOff()
{
	//отключение реле К1; Контактор удерживается реле K4;Реле К4-удерживается сигналом RUN от УПП
	if(isK1on == 0) return 0;
	GPIO_SetBits(GPIO_PORT_STOP, GPIO_PIN_STOP);
	for(int i=0; i<1000000; i++);
	GPIO_ResetBits(GPIO_PORT_STOP, GPIO_PIN_STOP);
	isK1on == 0;
	return !contactorStatus();
}

uint8_t contactorStatus()
{
	for(int jh=0; jh<100000; jh++);
	return GPIO_ReadInputDataBit(gpioPortStatusWork, gpioPinStatusWork);
}
/////////////////////////////END CONTACTOR/////////////////////////////////////////////////////////

//*Конец Изменено в связи с добавлением 3-го порта 01.07.19

////////////////////////////////////ADDING, HELPING/////////////////////////////////////////

void UPPchoice()
{
	if(g_sConfig.FirstUPP == FIRST_UPP_LEFT)
	{
		g_UPPstruct = &UPP_LEFT;
		g_SecondUPP = &UPP_RIGHT;

	}
	else
	{
		g_UPPstruct = &UPP_RIGHT;
		g_SecondUPP = &UPP_LEFT;
	}
}

void equipment_config()
{
	g_WLS.NA = (uint8_t)g_sConfig.WLS_NA;
	g_WLS.rqstTry = 0;
	g_WLS.respSts = 0;
	g_WLS.minLvl = 1;
	g_WLS.maxLvl = 0;
	g_WLS.lastRespTime = 0xFFFFFFFF;
	UPP_LEFT.NA = (uint8_t)g_sConfig.UPP_NA_LEFT;
	UPP_LEFT.engineSts = 0;
	UPP_LEFT.rqstTry = 0;
	UPP_LEFT.respSts = 0;
	UPP_LEFT.lastRespTime = 0xFFFFFFFF;
	UPP_RIGHT.NA = (uint8_t)g_sConfig.UPP_NA_RIGHT;
	UPP_RIGHT.engineSts = 0;
	UPP_RIGHT.rqstTry = 0;
	UPP_RIGHT.respSts = 0;
	UPP_RIGHT.lastRespTime = 0xFFFFFFFF;
}

void controlUnitRqst()
{
 	uint8_t data[2]={241, 0x00};
 	uint8_t fun = 0x4F;
 	int8_t lengthWait = -1;
 	MasterModbusSend(242, fun, data, sizeof(data), lengthWait, 3, 1);
}

////////////////////////////////END REQUEST'S/////////////////////////////////////////

////////////////////////////////////MODBUS SEND/////////////////////////////////////////////
uint8_t MasterModbusSend(uint8_t NA, uint8_t fun, uint8_t* data, uint8_t len, int8_t lengthWait, uint8_t tryes, uint8_t timeout) //TODO доделать для длинных адресов
{
	DELAY50000;
	IWDG_ReloadCounter();
	req.NA= NA;
	req.fun= fun;
	req.lengthWait =lengthWait;

	req.respSts=0;

	uint8_t gg_fullNA = 0;
	//if((len > 0) && (data == NULL)) return;

	uint8_t fdata[256];
	memset(fdata, 0, 256);

	uint16_t offset;
	uint16_t flen = 1+1+len+2;    //1(адрес)+1(номер функции)+ len (сами данные) +2 (crc)


	//printf("MasterModbusSend NA=%d \n\r",NA);
	if(gg_fullNA)
	{
		uint8_t i;
		fdata[0] = 251; //0xFB
		for(i = 0; i < 3; i++)
		{
			memrcpy_u32(&fdata[1 + i * 4], gc_Unique_ID[i]);
		}

		offset = 12;
		flen += 12;
	} else
	{
#ifdef __CONFIG_H__
		fdata[0]=g_sConfig.u8NetworkAddress;
#else
		fdata[0]=NA;
#endif
		offset = 0;
	}
	fdata[offset] = NA;
	fdata[offset + 1] = fun;

	if(data != NULL)
	{
		memcpy(fdata + offset + 2, data, len);
	}

	uint16_t gcrc = crc16Modbus(fdata, flen-2);
	fdata[flen-2]=LOW(gcrc);
	fdata[flen-1]=HIGH(gcrc);

	g_isAnswer= 0;

	//printf("MasterModbusSend flen=%d\n\r",flen);
	if(g_sConfig.u8NetworkAddress == 0)
		printf("g_sConfig.u8NetworkAddress == %d \n\r",g_sConfig.u8NetworkAddress);
	while( (tryes--) && (!g_isAnswer) )
	{
		UART4_send(fdata, flen); //46.232.165.21

		//printf("o1:");for(int jj=0;jj<flen;jj++)	printf("%2x ",fdata[jj]);printf("\n\r");

		//if(!g_isAnswer)
		wait_answer(timeout);
	}

	return g_isAnswer;
}
////////////////////////////////END MODBUS SEND/////////////////////////////////////////////

////////////////////////////////////MODBUS SEND_USART2/////////////////////////////////////////////
PUBLIC uint8_t MasterModbusSendUsart2(uint8_t NA, uint8_t fun, uint8_t* data, uint8_t len, int8_t lengthWait, uint8_t tryes, uint8_t timeout) //TODO доделать для длинных адресов
{
	//DELAY50000;
	IWDG_ReloadCounter();
	req.NA= NA;
 	req.fun= fun;
 	req.lengthWait =lengthWait;

 	req.respSts=0;

	uint8_t gg_fullNA = 0;
	//if((len > 0) && (data == NULL)) return;

	uint8_t fdata[256];
	memset(fdata, 0, 256);

	uint16_t offset;
	uint16_t flen = 1+1+len+2;    //1(адрес)+1(номер функции)+ len (сами данные) +2 (crc)


	if(gg_fullNA)
	{
		uint8_t i;
		fdata[0] = 251; //0xFB
		for(i = 0; i < 3; i++)
		{
			memrcpy_u32(&fdata[1 + i * 4], gc_Unique_ID[i]);
		}

		offset = 12;
		flen += 12;
	} else
	{
#ifdef __CONFIG_H__
		fdata[0]=g_sConfig.u8NetworkAddress;
#else
		fdata[0]=NA;
#endif
		offset = 0;
	}
	fdata[offset] = NA;
	fdata[offset + 1] = fun;

	if(data != NULL)
	{
		memcpy(fdata + offset + 2, data, len);
	}

	uint16_t gcrc = crc16Modbus(fdata, flen-2);
	fdata[flen-2]=LOW(gcrc);
	fdata[flen-1]=HIGH(gcrc);

	g_isAnswer= 0;

	while( (tryes--) && (!g_isAnswer) )
	{
		Usart2_send(fdata, flen); //46.232.165.21
		//printf("%d o2:",RTC_GetCounter());for(int jj=0;jj<flen;jj++)	printf("%2x ",fdata[jj]);
		//printf("\n\r");
		wait_answer2(1);
	}
	return g_isAnswer;
}
////////////////////////////////END MODBUS SEND/////////////////////////////////////////////
////////////////////////////////////PARSING'S/////////////////////////////////////////

void reqUPPTo_CONTROL_BY_SERVER()
{
	if(gControlMode == CONTROL_BY_SERVER)
	{
		UPPstatusRqst(g_UPPstruct);
	}
}

void wait_answer(uint8_t timeout)
{
//-printf("%10d B :wait(%d)\n\r ", RTC_GetCounter(),timeout);

	//Запуск таймера 4
	g_secound=0;
	TIM_Cmd(TIM4, ENABLE);
	while( (g_secound < timeout) && (!g_isAnswer))
	{
		led5blink();
	}
	TIM_Cmd(TIM4, DISABLE);

	//-printf("%10d: E wait(%d)\n\r ", RTC_GetCounter(),timeout);

	for(int jk=0; jk<1000; jk++);
}
void wait_answer2(uint8_t timeout)
{
	g_timeout2 = 1;
	TIM2->ARR = 1000 * timeout;
	TIM_Cmd(TIM2, ENABLE);
	while(g_timeout2 && (!g_isAnswer))
	{
		led5blink();
	}
	//TIM_Cmd(TIM2, DISABLE);
}
void wait_(uint8_t timeout)
{
	g_secound = 0;
	gWatchDog = 0;
	TIM_Cmd(TIM4, ENABLE);
	while( (g_secound < timeout) )
	{
		led5blink();
	}
	TIM_Cmd(TIM4, DISABLE);

	for(int jk=0; jk<1000; jk++);
}


void led5blink()
{
	if(GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_5))
	{
		GPIO_ResetBits(GPIOA, GPIO_Pin_5);
	}else
	{
		GPIO_SetBits(GPIOA, GPIO_Pin_5);
	}
}

u8 CalcCRC8FromChId(){
	uint8_t chId[12];
	for(u8 j = 0; j < 3; j++)
	{
		memrcpy_u32(&chId[j * 4], gc_Unique_ID[j]);
	}
	return Crc8(chId, 12);
}

void ReadFromBKP()
{
	g_UPPstruct->lastStartTime = BKP_ReadBackupRegister(BKP_FIRST_LAST_START2)<<16 | BKP_ReadBackupRegister(BKP_FIRST_LAST_START1);
	g_UPPstruct->lastStopTime = BKP_ReadBackupRegister(BKP_FIRST_LAST_STOP2)<<16 | BKP_ReadBackupRegister(BKP_FIRST_LAST_STOP1);

	g_SecondUPP->lastStartTime = BKP_ReadBackupRegister(BKP_SECOND_LAST_START2)<<16 | BKP_ReadBackupRegister(BKP_SECOND_LAST_START1);
	g_SecondUPP->lastStopTime = BKP_ReadBackupRegister(BKP_SECOND_LAST_STOP2)<<16 | BKP_ReadBackupRegister(BKP_SECOND_LAST_STOP1);
}


void WriteToBKPLastStartTime(softStarter* Upp)
{
	if(Upp->NA == g_UPPstruct->NA)
	{
		BKP_WriteBackupRegister(BKP_FIRST_LAST_START1, Upp->lastStartTime & 0xFFFF);
		BKP_WriteBackupRegister(BKP_FIRST_LAST_START2, Upp->lastStartTime >> 16);
	}
	else
	{
		BKP_WriteBackupRegister(BKP_SECOND_LAST_START1, Upp->lastStartTime & 0xFFFF);
		BKP_WriteBackupRegister(BKP_SECOND_LAST_START2, Upp->lastStartTime >> 16);
	}
}

void WriteToBKPLastStopTime(softStarter* Upp)
{
	if(Upp->NA == g_UPPstruct->NA)
	{
		BKP_WriteBackupRegister(BKP_FIRST_LAST_STOP1, Upp->lastStopTime & 0xFFFF);
		BKP_WriteBackupRegister(BKP_FIRST_LAST_STOP2, Upp->lastStopTime >> 16);
	}
	else
	{
		BKP_WriteBackupRegister(BKP_SECOND_LAST_STOP1, Upp->lastStopTime & 0xFFFF);
		BKP_WriteBackupRegister(BKP_SECOND_LAST_STOP2, Upp->lastStopTime >> 16);
	}
}
#endif
