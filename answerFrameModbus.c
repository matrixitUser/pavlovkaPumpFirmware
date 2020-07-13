#include "answerFrameModbus.h"

#define TIME_CORRECTION_MAX_SEC		120
#define MAX_REG_BYTES				180

PRIVATE void answerFrameProcess(uint8_t NA, uint8_t* frame, uint16_t len)
{
	uint8_t i;
	uint8_t fun = frame[0];
	uint8_t lengthWait = len-2-2; //?
	if(g_MonitoringSet)
	{
		sendTerminal(NA, frame, len, 1);
	}

	//����� �� ����������
	if(gControlMode == CONTROL_BY_AUTO)
	{
		if( (NA == req.NA) && (fun == req.fun) && ((req.lengthWait == -1 )||(req.lengthWait == lengthWait) )) //��� ����� �� ���� �������
		{
			//_printf(2, "Responce received na=%i, fun=%i, length=%i|\n\r",NA, frame[0],len);
			g_isAnswer = 1;
			for(i=0;i<len;i++) {g_frame[i] = frame[i]; g_frameLen = len;}
		}
		else if((fun) == (req.fun+128))
		{
			//TODO �������� ������ (�� ������)
			//_printf(2, "NOT RIGHT COMMAND|\n\r");
		}
		req.respSts=0;
	}
	if(gControlMode == CONTROL_BY_SERVER)
	{
		uint8_t flen = len+1+2;
		uint8_t fdata[256];
		memset(fdata, 0, 256);

		int funNAT=MODBUS_USER_FUNCTION_TRANSFER_TO_USART1;
		fdata[0] = NA;
		for(int i=0; i < flen-2; i++)
		{
			fdata[i+1] = frame[i];
		}
		uint16_t gcrc = crc16Modbus(fdata, flen-2);
		fdata[flen-2]=LOW(gcrc);
		fdata[flen-1]=HIGH(gcrc);
		FrameModbus_Send(funNAT, fdata, flen);
		//Usart3_send(fdata, flen);
		//TODO �������� ����� �� 50-�� ������� � ��������� Usart3_Send
   //����� �� ���, �������� ���������� �������
	}
}

//*������ �������� � ����� � ����������� 3-�� ����� 01.07.19
PUBLIC void answerFrameModbusDataProcess(uint8_t *data, uint16_t len)
{
	//printf("answerFrameModbusDataProcess len=%d \n\r",len);
	uint8_t fullNA = 0;
	uint8_t chipid[12];
	for(int i=0; i<12; i++)
	{
		chipid[i] = 0;
	}
	if(len < 1) return; //*01.07.19

	uint8_t head = 1;   //������ ���� ����� ������
	//��������� ������
	if(data[0] == 251) //0xFB additional header w/ chipID
	{
		fullNA=1;
		if(len < (4+12)) return;//?
		int i;
		for(i=0; i<12; i++)
		{
			if(chipid[i] != data[1+i]) return;
		}

		head = 1+12;  //������ ���� ����� ������ ?
	}
	else
	{
		chipid[0] = data[0];
		fullNA=0;
	}
	uint16_t ccrc = ((u16)data[len-1]<<8) | data[len-2]; //?
	uint16_t gcrc = crc16Modbus(data, len-2);//?

	if(gcrc == ccrc)
	{
		//uint16_t NAcrc = crc16Modbus(chipid, 12);//?crcNA
		answerFrameProcess(data[0], data+head, len-(2+head));
	}
	//printf("i1:");for(int jj=0;jj<len;jj++)	printf("%2x ",data[jj]);printf("\n\r");//��� ���� � 2?��
}

void sendTerminal(uint8_t NA, uint8_t* frame, uint16_t len, uint8_t sendOrReceive)
{
	DELAY50000;
	//_printf(2," === sendMesagge  ==");
	uint8_t ret[100];
 	//----------------------- ����� ��������� �� ����  BEGIN  ------------------//
 	//����� ��� ����: Object_id-16,����, NA=1-����,event Or error-1,g_eventCode-2,time-4
	for(int i = 0;i < 16; i++)    //14 ,FQ
	{
		ret[i] = g_objectId[i];
	}

    ret[16] = g_sConfig.u8NetworkAddress;     //30 ����
    ret[17] = sendOrReceive; //0-error;1 -event

    ret[18] = 0xFF;    //32
    ret[19] = 0xFF;
    u32 time = RTC_GetCounter();

	uint8_t bytes[4];
	memcpy(bytes,&time,4);
	ret[20] = bytes[0];  //34
	ret[21] = bytes[1];
	ret[22] = bytes[2];
	ret[23] = bytes[3];
	//----------------------- ����� ��������� �� ����  END  ------------------//
	ret[24] = NA;
	for(int i=0; i<len; i++)
	{
		ret[i+25] = frame[i];
	}

	uint8_t fullNA = g_fullNA;  g_fullNA = 1;
    FrameModbus_Send(MODBUS_DEBUG_TERMINAL, ret,len+25);
    g_fullNA = fullNA;
}


void printff(char* frame, uint16_t len)
{
	DELAY50000;
	uint8_t ret[100];
 	//----------------------- ����� ��������� �� ����  BEGIN  ------------------//
 	//����� ��� ����: Object_id-16,����, NA=1-����,event Or error-1,g_eventCode-2,time-4
	for(int i = 0;i < 16; i++)    //14 ,FQ
	{
		ret[i] = g_objectId[i];
	}

    ret[16] = g_sConfig.u8NetworkAddress;     //30 ����
    ret[17] = 2; //0-error;1 -event 3-printf

    ret[18] = 0xFF;    //32
    ret[19] = 0xFF;
    u32 time = RTC_GetCounter();

	uint8_t bytes[4];
	memcpy(bytes,&time,4);
	ret[20] = bytes[0];  //34
	ret[21] = bytes[1];
	ret[22] = bytes[2];
	ret[23] = bytes[3];
	//----------------------- ����� ��������� �� ����  END  ------------------//
	ret[24] = 239;
	for(int i=0; i<len; i++)
	{
		ret[i+25] = frame[i];
	}

	uint8_t fullNA = g_fullNA;  g_fullNA = 1;
    FrameModbus_Send(MODBUS_DEBUG_PRINFF, ret,len+25);
    g_fullNA = fullNA;
}

