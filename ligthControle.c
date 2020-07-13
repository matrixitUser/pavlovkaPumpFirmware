#ifdef LIGHT_CONTROLE
#include "ligthControle.h"
#include "time.h"

static uint16_t errorCode = 0;
static uint16_t eventCode = 0;
static uint8_t tacts = 0;
static uint8_t tactsEvent = 0;
static uint8_t iArchive = 0;
static u32 manual_begin; //����� ������ ������ � ����� ������� ����������
static u32 HandModeStart1; //����� ������ ������ � ����� ������� ���������� ����� 1
static u32 HandModeStart2; //����� ������ ������ � ����� ������� ���������� ����� 2
static u32 HandModeStart3; //����� ������ ������ � ����� ������� ���������� ����� 3
static u32 HandModeStart4; //����� ������ ������ � ����� ������� ���������� ����� 4




static tryes = 0;  //�������� ���������� �������� ��������� (�������, ���� ������������ ��������� �����)
static tryesEvent = 0;  //�������� ���������� �������� ��������� (�������, ���� ������������ ��������� �����)
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
  GPIO_InitStructure.GPIO_Pin   =  GPIO_PIN_LIGHT2 | GPIO_PIN_LIGHT3 | GPIO_PIN_LIGHT4 | GPIO_PIN_12V_OUT5 | GPIO_PIN_12V_OUT6; //������ ����
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
  GPIO_InitStructure.GPIO_Pin   =  GPIO_PIN_LIGHT1; //������ ����
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  setOutPins();

  //IN
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPD;
  GPIO_InitStructure.GPIO_Pin   = GPIO_PIN_CONTACTOR_IN1 | GPIO_PIN_CONTACTOR_IN2 | GPIO_PIN_CONTACTOR_IN3 | GPIO_PIN_CONTACTOR_IN4 | GPIO_PIN_12V_IN5; //����� 220v AIN1
  GPIO_Init(GPIOB, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPD;
  GPIO_InitStructure.GPIO_Pin   = GPIO_PIN_12V_IN6; //����� 220v AIN1
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
	turn=1-turn; if(turn)  event(); else error();  //� ������ ������ ���������� ������������ ������� ������ � �������
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
	if(year < 2019) //���� �� ����������� ��� ��������, ����������  ��������� � ����� �� �����������
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

//������������ ����������
void shedulle(u8 hdl, u32 time)
{

	long ltime = (long) time;
	long itime =  gmtime(&itime);
	uint8_t u8Action = g_stateLight;
	uint8_t onOff;

	struct tm* currenTime = gmtime(&ltime);
	int currenTimeInSec  = 60*((currenTime -> tm_hour)*60 + currenTime -> tm_min);  //������� ����� � �������� �� ������ �����
	uint8_t iChannel=0;
	volatile uint8_t iShedulle=0;
    for (iChannel = 1; iChannel <=4 ; iChannel++) //� ������ � ���������� ������������� � 1
    {
    	if( (g_sControlMode[iChannel]  == CONTROLE_BY_SHEDULLE) || (g_sControlMode[iChannel]  == CONTROLE_BY_ASTR_SHEDULLE ) )
    	{
            for (iShedulle = 1; iShedulle <=2 ; iShedulle++)
            {
            	if((g_sOn[iChannel][iShedulle] != TIMESTAMP_UNDEFINED ) && (g_sOff[iChannel][iShedulle] != TIMESTAMP_UNDEFINED ))
            	{

             		long OnSec  = g_sOn[iChannel][iShedulle];   //����� ��������� � ������� �� ������ �����
            		long OffSec = g_sOff[iChannel][iShedulle];  //����� ���������� � ������� �� ������ �����

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
            		}else //���������� �� ��������� ������� ���.������ ����� � ������� ���� ������ ������� ���������, ���� ���
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
                	//���� ������.������ ������� ��� ������, �� ��������� �� ����� ��������
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

//���/���� ���������
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

	//����������� �������: GMT + ������� ����
	g_sunRise= sunRiseGMT + g_sConfig.u8timeDiff;
	g_sunSet = sunSetGMT  + g_sConfig.u8timeDiff;
	if(g_sunRise>=24.0) g_sunRise-=24.0;
	if(g_sunSet>=24.0) g_sunSet-=24.0;

	//����� 1
	//����� ���������� ��������� � ������ ���������� (����� ���� �������������, ���� ���������� �����)
	g_RiseTime[1] = g_sunRise -  g_sConfig.ligthtChannels1.u8beforeSunRise/60.0;
	//����� ��������� ��������� � ������ �������� (����� ���� ������ ��� ����� 24, ���� ����������� �����)
	g_SetTime[1] = g_sunSet + g_sConfig.ligthtChannels1.u8afterSunSet/60.0;

	//����� 2
	g_RiseTime[2] = g_sunRise -  g_sConfig.ligthtChannels2.u8beforeSunRise/60.0;
	g_SetTime[2] = g_sunSet + g_sConfig.ligthtChannels2.u8afterSunSet/60.0;
	//����� 3
	g_RiseTime[3] = g_sunRise -  g_sConfig.ligthtChannels3.u8beforeSunRise/60.0;
	g_SetTime[3] = g_sunSet + g_sConfig.ligthtChannels3.u8afterSunSet/60.0;
	//����� 4
	g_RiseTime[4] = g_sunRise -  g_sConfig.ligthtChannels4.u8beforeSunRise/60.0;
	g_SetTime[4] = g_sunSet + g_sConfig.ligthtChannels4.u8afterSunSet/60.0;

}

uint8_t set_by_astr_time(u32 time,uint8_t Channel)
{

	// ��������������� ������ ( sunriset) ������ ����� ������� � ������ �� �����
	// �������� ����������� � ����� ���������  �.�. ������� ��������� (��� ��� ����):
	// 1.����� ������� (������ ��,��) ����� ������ ������� ���������� --> "������-"  ���������� g_RiseTime
	// 2.���� ������ ���� �� �������  (��� ���������� ����)
	// 3.����� ������  (������ ��,��) ���� ������ �������� ���������� --> "�����+"  ���������� g_SetTime
	// 4.���� ������ 24 �� �������  (��� ��������� ����)
	// 5. �.� ���������, ���� ������� ����� ���������  ����� "������-" � "�����+"
	// 6. ����� �.� ��������
    // ��������� sunriset ��� ������� �������� , ��� �� 	"������-" >= "�����+"? (������� �� ����������)


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

//���������� ��������� ��������
void monitoring(u8 hdl, u32 time)
{

	long ltime = (long) time;
	struct tm* currenTime = gmtime(&ltime);
	int   year = currenTime->tm_year+1900;
	int   month = currenTime->tm_mon +1;
	int   day = currenTime->tm_mday;

	//������������ ����� ���������� � ������ ������
	//controlHandMode();

	//��������� ����� �������
	if( year < 2019 )//TODO
	{
		SETBIT(g_errorCode,ERR_TIMER);

		if(g_sConfig.ligthtChannels1.u8ControlMode == CONTROLE_BY_ASTR_TIMER)
		{
			if(g_sConfig.ligthtChannels1.u8ControlMode != CONTROLE_BY_FHOTOSENSOR) //���� ��� �� � ������ �����������
			{
				if(g_sConfig.u8hardware & 0x01) //���� �������� ����������,  �� ���� ���������� � ��������������
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

	//�������� ������������ ���� � ������������� ����� ���������, � ����������� ������ ������
	uint8_t iChannel;
	for (iChannel= 1; iChannel <= 4; iChannel++)
	{
		if( (g_sControlMode[iChannel] == CONTROLE_BY_ASTR_TIMER) || (g_sControlMode[iChannel] == CONTROLE_BY_ASTR_SHEDULLE) )
		{
    		g_astrLight = set_by_astr_time(time,iChannel);
    		set_ligthByChannel(iChannel, g_astrLight);
		}
	}

	//*TODO ��� ���� �������
	//��������� ��������� �������� ������� ���������� � ��������� ������� �����������
	if(GPIO_ReadInputDataBit(GPIO_PORT_CONTACTOR_IN, GPIO_PIN_CONTACTOR_IN) != GPIO_ReadOutputDataBit(GPIO_PORT_LIGHT, GPIO_PIN_LIGHT))
	{
		SETBIT(g_errorCode, ERR_OUT_MCU_AND_REAL);
		//TODO ���������� ������������ ��������
	}
	else
	{
		CLRBIT(g_errorCode, ERR_OUT_MCU_AND_REAL);
	}


	//��������� ������������
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

//�������� ��������� ��������� ����� �� �����������
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
	//�������� ��� ������
	if( (g_countTry) && (g_errorCode > 0))    	//g_count_try - ���������� ������� �������� ������ � ���� �� ����� ��������� (������) ,
	{

		if(g_errorCode == errorCode)
		{
			if(g_timeErrorMessage != timeErrorMessage)
			{
				sendErrorTry();
			}
			else  //������ ���������� � �������
			{
				//g_errorCode = 0 ; ������ �������� (������  �� ����� �� ��������)
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
		tryes=0;  //��������� ����� g_countTryEvent = 3 ������� ��������� ���� � �� �� ������
	}
}

void sendErrorTry()
{

	if((g_errorCode == errorCode))  //errorCode  ������������� �������� g_errorCode ����� �������� ( � sendError)
	{
		//��� ����� ������, � ��� ��� ����������-> �����:
		//���������� g_count_tacts ���-�� �������
		tacts++;
		if(tacts * MONITORING_TIME < g_errorSendInterval) return; //�������� ����� ���������� ���������� ����� ��������� ������), ���
	}
	else
	{
		tryes = 0;  // ��� ��������� ���� ������
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
	//�������� �������
	if( (g_countTryEvent) && (g_eventCode > 0))
	{
		if((g_eventCode == eventCode))  //��������� ������� �������
		{
			if(g_timeEventMessage != timeEventMessage)
			{
				sendEventTry();
			}
			else  //������� ���������� � �������
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
		tryesEvent=0;  //��������� ����� g_countTryEvent = 3 ������� ��������� ���� � �� �� ��������
	}
}


void sendEventTry()
{

	if((g_eventCode == eventCode))  //errorCode  ������������� �������� g_errorCode ����� �������� ( � sendError)
	{
		//��� ����� ������, � ��� ��� ����������-> �����:
		//���������� g_count_tacts ���-�� �������
		tactsEvent++;
		if(tactsEvent * MONITORING_TIME < g_errorSendInterval) return; //�������� ����� ���������� ���������� ����� ��������� ������), ���
	}
	else
	{
		tryesEvent = 0;  // ��� ��������� ���� ������
	}

	if(tryesEvent < g_countTry)
	{
		sendEvent();
		eventCode = g_eventCode;
		tactsEvent = 0;
		tryesEvent ++;
	}
}

void sendEvent()  //������� ���� - ������������ ������� ����������; ������� - ���/���� ����� ��.�
{
	//if(USART_GetITStatus(USART1, USART_IT_IDLE))  //��������� �����
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
	//����� 1
	//��������� ����� � ������� �������� � ������ �����
	if ((g_sConfig.ligthtChannels1.u8ControlMode & CONTROLE_BY_SERVER) == 0x0000) // �� ��������� � ������ ������� ����������,
	{
		HandModeStart1= time;										// ���������� ����� ���������� ��c������ �� � ������ ������
		CLRBIT(g_errorCode,ERR_HANDMODE_TIMEOUT);
	}
	else
	{
		//�������� �������, ���������� � ������ �������� � ������ �����. g_time_manual- ������������ ����� ���������� � ������ �������
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

	//����� 2
	//��������� ����� � ������� �������� � ������ �����
	if ((g_sConfig.ligthtChannels2.u8ControlMode & CONTROLE_BY_SERVER) == 0x0000) // �� ��������� � ������ ������� ����������,
	{
		HandModeStart2 = time;										// ���������� ����� ���������� ��c������ �� � ������ ������
		CLRBIT(g_errorCode,ERR_HANDMODE_TIMEOUT);
	}
	else
	{
		//�������� �������, ���������� � ������ �������� � ������ �����. g_time_manual- ������������ ����� ���������� � ������ �������
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

	//����� 3
	//��������� ����� � ������� �������� � ������ �����
	if ((g_sConfig.ligthtChannels3.u8ControlMode & CONTROLE_BY_SERVER) == 0x0000) // �� ��������� � ������ ������� ����������,
	{
		HandModeStart1 = time;										// ���������� ����� ���������� ��c������ �� � ������ ������
		CLRBIT(g_errorCode,ERR_HANDMODE_TIMEOUT);
	}
	else
	{
		//�������� �������, ���������� � ������ �������� � ������ �����. g_time_manual- ������������ ����� ���������� � ������ �������
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

	//����� 4
	//��������� ����� � ������� �������� � ������ �����
	if ((g_sConfig.ligthtChannels4.u8ControlMode & CONTROLE_BY_SERVER) == 0x0000) // �� ��������� � ������ ������� ����������,
	{
		HandModeStart4 = time;										// ���������� ����� ���������� ��c������ �� � ������ ������
		CLRBIT(g_errorCode,ERR_HANDMODE_TIMEOUT);
	}
	else
	{
		//�������� �������, ���������� � ������ �������� � ������ �����. g_time_manual- ������������ ����� ���������� � ������ �������
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


