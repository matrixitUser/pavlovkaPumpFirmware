#include "global.h"

PRIVATE const char unit0[] = {0x00};
PRIVATE const char unit1[] = {0xd0, 0xba, 0xd0, 0x92, 0xd1, 0x82, 0x2a, 0xd1, 0x87, 0x00};//���*�
PRIVATE const char unit2[] = {0xd0, 0xba, 0xd0, 0xb2, 0xd0, 0xb0, 0xd1, 0x80, 0x2a, 0xd1, 0x87, 0x00};//����*�
PRIVATE const char unit3[] = {0xd0, 0xba, 0xd0, 0x92, 0xd1, 0x82, 0x00};//���
PRIVATE const char unit4[] = {0xd0, 0xba, 0xd0, 0xb2, 0xd0, 0xb0, 0xd1, 0x80, 0x00};//����
PRIVATE const char unit5[] = {0xd0, 0xbc, 0x33, 0x00};//�3
PUBLIC const char* gc_pacUnits[UNITS_TOTAL] = {unit0, unit1, unit2, unit3, unit4, unit5};
PUBLIC uint8_t g_accessLevel;
PUBLIC tsConfig g_sConfig;
PUBLIC const uint32_t *gc_Unique_ID = (uint32_t *)0x1FFFF7E8;

PUBLIC tsEvent g_event[ARCHIVE_SIZE];

PUBLIC void Global_InitVars(void)
{
	g_accessLevel = ACCESS_LEVEL_NONE;
	memset(&g_sConfig, 0, sizeof(tsConfig));
}

//Abnormal states and settings(��������� ��������� �������� � ���������)

//PUBLIC u32 g_manualBegin;//����� ������ �������� � ������ �����

//----------------------  SEND EVENT AND ERROR -------------------------------------//
PUBLIC uint8_t g_countTry=      3; //���������� ������� �������� ������ � ���� �� ����� ��������� (������) , ��� 0 - �� ������������
PUBLIC uint8_t g_countTryEvent =3; //���������� ������� �������� ������ � ���� �� ����� ��������� (�������) , ��� 0 - �� ������������
PUBLIC uint8_t g_restart = 0;
PUBLIC uint8_t g_errorSendInterval = 3 * MONITORING_TIME ; //�������� ����� ���������� ���������� ����� ��������� ������

PUBLIC uint16_t g_errorCode = 0;
PUBLIC uint16_t g_eventCode = 0;

PUBLIC uint8_t g_objectId[16]={0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};


#ifdef LIGHT_CONTROLE

PUBLIC uint8_t  g_stateLight=0;
PUBLIC uint8_t g_state_photosensorIn; 	//1/0- ���������� ��������� ������ �� ���������/����������
PUBLIC uint8_t g_state_photosensorOut; 	//1/0- ������/��������� ���������� �� ����������

//PUBLIC uint8_t g_shedulle;    // ������� �������� ����������
PUBLIC uint8_t g_do_lightAction; // ��������� �������� �� ���������� (���/����)

//g_errorCode ->      1 ���-���� �������, 2 ���-�������������� ��������� ����� ���������������� �������,
				//3 ���-������������� ������� ������ ���������� ����������� � ��������� ������������ ���������,
				//4 ���-���������� ������� ���������� � ������ ������ 5,6,7
				//8 ���-������������


PUBLIC uint8_t g_MaxTimeManual = 30; //����� ���������� � ������ ������, ���
//sunrise ��������������� ������
PUBLIC double g_lat =  56.2751000; // �����   55.9077-�������;
PUBLIC double g_lon = 54.9338000;  // �����   53.9355-�������;
PUBLIC double g_timeDiff = 5.0;
PUBLIC double g_sunRise = 0.0;
PUBLIC double g_sunSet =  0.0;

PUBLIC double g_RiseTime[5] = {8.0,8.0,8.0,8.0,8.0};
PUBLIC double g_SetTime[5] = {20.0,20.0,20.0,20.0,20.0};

#endif


PUBLIC uint8_t g_isArchive = 0;     //��� g_isArchiv� > 0 - ����� �����

PUBLIC uint8_t g_fullNA = 0;

PUBLIC uint32_t g_Event_timeMessageSend=1;
PUBLIC uint32_t g_Event_timeMessageReceive;
PUBLIC uint32_t g_Error_timeMessageSend=1;
PUBLIC uint32_t g_Error_timeMessageReceive;


//��� �������
PUBLIC uint8_t g_debug = 0;
PUBLIC uint16_t g_timeout;

PUBLIC uint8_t g_writeConfig=0;

PUBLIC uint8_t g_sControlMode[5];
PUBLIC uint32_t g_sOff[5][3];
PUBLIC uint32_t g_sOn[5][3];
PUBLIC int8_t g_astrLight = -1;

//////////////////////////////��������/////////////////////
PUBLIC uint8_t  gPrintf = 0;

PUBLIC int8_t g_WLSsetLvl = 3;
PUBLIC uint8_t g_MonitoringSet = 0;

PUBLIC levelSensor g_WLS; //��������� ������ ������� ������ ����
PUBLIC softStarter UPP_LEFT; //��������� ������ ���1
PUBLIC softStarter UPP_RIGHT; //��������� ������ ���2

PUBLIC softStarter* g_UPPstruct = &UPP_LEFT;
PUBLIC softStarter* g_SecondUPP = &UPP_RIGHT;

PUBLIC uint8_t g_leftUppActivity = 0;//LEFT_UPP_ACTIVITY = 0;
PUBLIC uint8_t g_rightUppActivity = 0;//RIGHT_UPP_ACTIVITY = 0;

PUBLIC uint32_t gpioPortStatusWork = GPIO_PORT_LEFT_STATUS_WORK;
PUBLIC uint32_t gpioPinStatusWork = GPIO_PIN_LEFT_STATUS_WORK;

PUBLIC uint8_t gTransferAllow; // ���������� �� �������� ������� ����� ������������� � ������
PUBLIC uint8_t gControlMode = 0;

//��������, ������� ������ 04.07.19
PUBLIC uint16_t g_errorMaster; // ������ ���, �������� �� �����: 0-�� �� �������, 1-���1 �� �������, 2-���2 �� �������, 3-���3 �� �������
PUBLIC uint16_t g_eventMaster; // ������� �� ���� ��������, ���

PUBLIC uint8_t g_Timer3=0; // ���� ���������� �� ������� TIM3
PUBLIC uint8_t g_Timer4=0; // ���� ���������� �� ������� TIM3

PUBLIC rqstStruct req; //��������� �������� ������

// ������ �������� ��������� ���
PUBLIC uint8_t g_forseToError = 0; //���� ���������� ������� 3
PUBLIC uint16_t secBeforeOn = 0; //������ �� ��������� ���

PUBLIC uint8_t nCommand=1;
PUBLIC uint8_t g_isAnswer;
PUBLIC uint8_t g_frameLen;
PUBLIC uint8_t g_secound;

PUBLIC uint8_t g_frame[40];
PUBLIC uint8_t gWatchDog =0;

PUBLIC uint8_t g_WLSserverMode = 0xFF;  // WLS �� �������: 0- �� �����������; 1- ������������, ���� �� ������� ��������
															 //2- ������ �� ������� (��������� ��������� ����/���� ���������
PUBLIC uint8_t g_WLSfromserver =0xFF; 		  //���� WLS �� �������,����� ����� ����������� ��� ���?

PUBLIC uint8_t gtimeout = 0;

PUBLIC u8 gFromUsart = 0;
PUBLIC u8 gCountNotAnswerController = 0;
PUBLIC u8 gTimerForTalking = 0;
PUBLIC u8 gIsRestartMC = 0;
PUBLIC u8 gIsAnswerMC = 0;
PUBLIC uint8_t g_timeout2 = 0;
PUBLIC uint8_t g_dataTalk[40];
tsCallback g_tsCb;
u8 gIsAccessSecondCrtl = 0;
u8 gCountNotAnswerPump = 0;
