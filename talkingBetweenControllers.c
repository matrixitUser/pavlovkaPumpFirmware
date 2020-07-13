#include "talkingBetweenControllers.h"

//*Начало Изменено в связи с добавлением
PUBLIC void AnswerControllers(uint8_t *data, uint16_t len)
{
	if(len < 5) return;
	if(data[0] != g_sConfig.u8NetworkAddress) return;

	uint16_t ccrc = ((u16)data[len-1]<<8) | data[len-2]; //?
	uint16_t gcrc = crc16Modbus(data, len-2);//?
	if(gcrc != ccrc) return;

	uint8_t fun = data[1];
	uint8_t cmd = data[2];

	uint8_t head = 1;   //Первый байт после адреса
	u8 funAndCmd = 2; //смещение на 2: fun = 1, cmd = 1 => 2
	//if(g_sConfig.u8Mode == MODE_UNKNOWN) g_sConfig.u8Mode = MODE_SLAVE;
	ParseDataByFunc(fun, cmd, data + head + funAndCmd, len-(head +funAndCmd));
	gCountNotAnswerController = 0;
	gIsRestartMC = 0;
}

void ParseDataByFunc(uint8_t fun, uint8_t cmd, uint8_t* frame, uint16_t len)
{
	switch(fun){
		case TALKING_BTWN_CONTROLLERS:
		{
			if(cmd == MODBUS_CMD_SEND)
			{
				long int u32Ts = RTC_GetCounter();
				long int u32Ts1 = (frame[3] << 24) | (frame[2] << 16) | (frame[1] << 8) | frame[0];

				//left upp
				long int UPP_LEFTLastRespTime = (frame[7] << 24) | (frame[6] << 16) | (frame[5] << 8) | frame[4];

				uint16_t UPP_LEFTStatus = (frame[8] << 8) | frame[9];

				long int UPP_LEFTLastStartTime = (frame[13] << 24) | (frame[12] << 16) | (frame[11] << 8) | frame[10];

				long int UPP_LEFTLastStopTime = (frame[17] << 24) | (frame[16] << 16) | (frame[15] << 8) | frame[14];

				//right upp
				long int UPP_RIGHTLastRespTime = (frame[21] << 24) | (frame[20] << 16) | (frame[19] << 8) | frame[18];

				uint16_t UPP_RIGHTStatus = (frame[23] << 8) | frame[22];

				long int UPP_RIGHTLastStartTime = (frame[27] << 24) | (frame[26] << 16) | (frame[25] << 8) | frame[24];

				long int UPP_RIGHTLastStopTime = (frame[31] << 24) | (frame[30] << 16) | (frame[29] << 8) | frame[28];

				//////////////////////////////////WLS
				//Время последнего опроса WLS
				long int g_WLSLastRespTime = (frame[35] << 24) | (frame[34] << 16) | (frame[33] << 8) | frame[32];

				//Байт состояния WLS
				uint16_t g_WLSStatus = (frame[36] << 8) | frame[37];

				//Байт состояния Датчика давления
				union
				{
					float float_variable;
					uint8_t outBytes[4];
				} hh;
				hh.outBytes[0]=frame[38];
				hh.outBytes[1]=frame[39];
				hh.outBytes[2]=frame[40];
				hh.outBytes[3]=frame[41];

				float g_WLSHeight = hh.float_variable;
				//controllers state
				u8 answerMode = frame[42];
				//printf("myMode=%d  answerMode=%d\n\r", g_sConfig.u8Mode, answerMode);
				//answer
				AnswerStateToCtrl(TALKING_BTWN_CONTROLLERS);
			}
			else
			{
				long int u32Ts1 = (frame[3] << 24) | (frame[2] << 16) | (frame[1] << 8) | frame[0];
				u8 answerMode = frame[4];
				//printf("myMode=%d  answerMode=%d \n\r", g_sConfig.u8Mode, answerMode);
			}
			break;
		}
		case TALKING_ASKING_WLS:
		{
			if(cmd == MODBUS_CMD_SEND)
			{
				TalkingCall(TALKING_ASKING_WLS);
				//tsCallback askingWlsCb = {cb: TalkingCallbackParse, hdl: TALKING_ASKING_WLS};
				//Callback_Save(&askingWlsCb);
			}
			else
			{
				//printf("---gIsAnswerMC= %d \n\r", gIsAnswerMC);
				for(u8 i=0;i<len;i++) {g_dataTalk[i] = frame[i];}
				gIsAnswerMC = 1;
			}
			break;
		}
		case TALKING_SWITCH_MASTER:
		{
			if(cmd == MODBUS_CMD_SEND)
			{
				TalkingCall(TALKING_SWITCH_MASTER);
			}
			else
			{
				if(frame[4] != MODE_MASTER) break;
				SwitchMasterSlave();
				gIsAnswerMC = 1;
			}
			break;
		}
		case TALKING_UPP_RQST:
		{
			if(cmd == MODBUS_CMD_SEND)
			{
				TalkingCall(TALKING_UPP_RQST);
			}
			else
			{

			}
			break;
		}
		case TALKING_UPP_START:
		{
			if(cmd == MODBUS_CMD_SEND)
			{

			}
			else
			{

			}
			break;
		}
		case TALKING_UPP_STOP:
		{
			if(cmd == MODBUS_CMD_SEND)
			{

			}
			else
			{
			}
			break;
		}
	}
}

void TalkingCall(u8 hdl)
{
	tsCallback uppRqstCb = {cb: TalkingCallbackParse, hdl: hdl};
	Callback_Save(&uppRqstCb);
}

void TalkingCallbackParse(u8 hdl, u32 param) // hdl === fun
{
	switch(hdl)
	{
		case TALKING_ASKING_WLS:
			AnswerForAskingWLS(myWLSstatusRqst());
			return;
		case TALKING_SWITCH_MASTER:
			SwitchMasterSlave();
			AnswerStateToCtrl(TALKING_SWITCH_MASTER);
			return;
		case TALKING_UPP_RQST:
			return;
		case TALKING_UPP_START:
			return;
		case TALKING_UPP_STOP:
			return;
	}
}

void SwitchMasterSlave()
{
	if(g_sConfig.u8Mode == MODE_SLAVE) g_sConfig.u8Mode = MODE_MASTER;
	else g_sConfig.u8Mode = MODE_SLAVE;
	//WriteConfig();
}

PUBLIC void AnswerForAskingWLS(u8 wlsOk)
{
	//printf("wlsOk= %d \n\r", wlsOk);
	uint8_t ret1[8];
	ret1[0] = wlsOk;
	if(wlsOk)
	{
		ret1[1] = g_WLS.status>>8;
		ret1[2] = g_WLS.status&0xFF;

		//answer
		union
		{
			float float_variable;
			u8 outBytes[4];
		} hh1;
		hh1.float_variable = g_WLS.height;
		printf("g_WLS.status = %d height=%d\n\r",g_WLS.status, (int)g_WLS.height*100);
		ret1[3]= hh1.outBytes[0];ret1[4]= hh1.outBytes[1];ret1[5]= hh1.outBytes[2];ret1[6]= hh1.outBytes[3];
	}
	else
	{
		for(u8 i = 1; i < sizeof(ret1); i++) ret1[i] = 0;
	}
	TalkingBtwnCtrls_Send(TALKING_ASKING_WLS, MODBUS_CMD_ANSWER, ret1, sizeof(ret1));
}

void ParseDataFromController(uint8_t* data){
	volatile uint8_t* dataControllers = data+1;
	uint16_t wls = ((dataControllers[0]<<8)|(dataControllers[1])); //0<<8 and wls
	uint8_t engineStsAnotherUpp = dataControllers[2];
}




PUBLIC u8 Talking_Send(uint8_t fun, uint8_t* data){
	uint32_t lTimeout = 10;
	TalkingBtwnCtrls_Send(fun, MODBUS_CMD_SEND, data, sizeof(data));
	while(lTimeout--){
		wait_(1);
		if(gIsAnswerMC){
			gIsAnswerMC = 0;
			return 1;
		}
	}
	return 0;
}

PUBLIC void TalkingBtwnCtrls_Send(uint8_t fun, uint8_t numCmd, uint8_t* data, uint8_t len)
{
	if((len > 0) && (data == NULL)) return;

	uint8_t fdata[256];
	memset(fdata, 0, 256);

	uint16_t flen = 1+1+1+len+2;    //1(адрес) + 1(номер функции) + 1(номер команды) + len (сами данные) +2 (crc)

	fdata[0] = g_sConfig.u8NetworkAddress;
	fdata[1] = fun;
	fdata[2] = numCmd;

	if(data != NULL)
	{
		memcpy(fdata + 3, data, len);
	}

	uint16_t gcrc = crc16Modbus(fdata, flen-2);
	fdata[flen-2]=LOW(gcrc);
	fdata[flen-1]=HIGH(gcrc);

	printf("send flen = %d; len = %d \n\r", flen, len);
	//for(int i = 0; i < flen; i++) printf("0x%x ",fdata[i]);
	Usart1_send(fdata, flen);
}

void AnswerStateToCtrl(u8 fun)
{
	uint8_t ret[5];
	long int u32Ts = RTC_GetCounter();
	memcpy(ret,&u32Ts,4);	//[0;3]
	ret[4] = g_sConfig.u8Mode;
	TalkingBtwnCtrls_Send(fun, MODBUS_CMD_ANSWER, ret, sizeof(ret));
}

PUBLIC void AckingWLSToMC(uint8_t *ret)
{
	for(u8 i = 0; i < 4; i++){
		ret[i] = 0;
	}
}

PUBLIC void ArrFillDATANULL(uint8_t *ret)
{
	for(u8 i = 0; i < 4; i++){
		ret[i] = 0;
	}
}

PUBLIC void CMDForTalkingBetweenControllers(uint8_t *ret){

	long int u32Ts = RTC_GetCounter();
	uint8_t bytes[4];
	memcpy(ret,&u32Ts,4);

	//left upp
	memcpy(bytes,&UPP_LEFT.lastRespTime,4);
	ret[4] = bytes[0];
	ret[5] = bytes[1];
	ret[6] = bytes[2];
	ret[7] = bytes[3];

	ret[8] = UPP_LEFT.status>>8;
	ret[9] = UPP_LEFT.status&0xFF;

	memcpy(bytes,&UPP_LEFT.lastStartTime,4);
	ret[10] = bytes[0];
	ret[11] = bytes[1];
	ret[12] = bytes[2];
	ret[13] = bytes[3];

	memcpy(bytes,&UPP_LEFT.lastStopTime,4);
	ret[14] = bytes[0];
	ret[15] = bytes[1];
	ret[16] = bytes[2];
	ret[17] = bytes[3];

	//right upp
	memcpy(bytes,&UPP_RIGHT.lastRespTime,4);
	ret[18] = bytes[0];
	ret[19] = bytes[1];
	ret[20] = bytes[2];
	ret[21] = bytes[3];

	ret[22] = UPP_RIGHT.status>>8;
	ret[23] = UPP_RIGHT.status&0xFF;

	memcpy(bytes,&UPP_RIGHT.lastStartTime,4);
	ret[24] = bytes[0];
	ret[25] = bytes[1];
	ret[26] = bytes[2];
	ret[27] = bytes[3];

	memcpy(bytes,&UPP_RIGHT.lastStopTime,4);
	ret[28] = bytes[0];
	ret[29] = bytes[1];
	ret[30] = bytes[2];
	ret[31] = bytes[3];
	//////////////////////////////////WLS

	//Время последнего опроса WLS
	memcpy(bytes,&g_WLS.lastRespTime,4);
	ret[32] = bytes[0];
	ret[33] = bytes[1];
	ret[34] = bytes[2];
	ret[35] = bytes[3];

	//Байт состояния WLS
	ret[36] = g_WLS.status>>8;
	ret[37] = g_WLS.status&0xFF;

	//Байт состояния Датчика давления
	union
	{
		float float_variable;
		uint8_t outBytes[4];
	} hh;
	hh.float_variable = g_WLS.height;
	ret[38]= hh.outBytes[0];ret[39]= hh.outBytes[1];ret[40]= hh.outBytes[2];ret[41]= hh.outBytes[3];

	//controllers state
	ret[42] = g_sConfig.u8Mode;

	if(!CheckConfigNew(&g_sConfig)) ReadConfig();
}
