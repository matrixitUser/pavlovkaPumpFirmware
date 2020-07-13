#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#define MONITORING_TIME 		15     //������ ������� ����������� ���������, ���

#define ERR_TIMER				0x00   //C������ ��� �� ���������� ������� �������
#define ERR_OUT_MCU_AND_REAL	0x02   //������������� ������� ������ ���������� ����������� � ��������� ������������ ���������
#define ERR_HANDMODE_TIMEOUT	0x03   //���������� ������� ���������� � ������ ������
#define ERR_RESTART				0x04   //������������
#define SIGNALING				0x10   //�������� ������������

#ifdef LIGHT_CONTROLE
//2 ���� ��������� (������)
#define ERR_ASTR_TIMER			0x01   //�������������� ��������� ������������ ��������� ���������������� �������,

//2 ����� ��������� ��������� �������!!!
#define TO_CONTROLE_BY_SHEDULLE		CONTROLE_BY_SHEDULLE   		//��������� ���������� �� ����������
#define TO_CONTROLE_BY_FHOTOSENSOR  CONTROLE_BY_FHOTOSENSOR		//��������� ���������� �� �����������
#define TO_CONTROLE_BY_SERVER       CONTROLE_BY_SERVER   		//��������� ���������� ��������
#define TO_CONTROLE_BY_SERVER_HARD  CONTROLE_BY_SERVER_HARD		//��������� ���������� ��������
#define TO_CONTROLE_BY_ASTR_TIMER 	CONTROLE_BY_ASTR_TIMER		//��������� ���������� �� ���������������� �������
#define TO_LIGTH_ON				     0x0100   //��������� ���������
#define TO_LIGTH_OFF				 0x0200   //���������� ���������

#endif

#ifdef PAVLOVKA
//2 ���� ��������� (������)
#define ERR_NOT_ACTIVITY_UPP				0x01   //C������ ��� �� ���������� ������� �������
#define ERR_UPP_NO_START					0x02   //�� ������� ��������� ���������
#define ERR_UPP_NO_STOP						0x03   //�� ������� ��������� ���������

#define ERR_WLS_NOT_RESPONCE				0x05   //��� ������ �� WLS(�����)
#define ERR_UPP_NOT_RESPONCE				0x06   //��� ������ �� UPP
#define ERR_WLS_SENSOR						0x07   //������ ������� ������ (��� ��������)
#define ERR_UPP_AUTOMODE_SET				0x08   //�� ������� ��������� ��� � �������
#define ERR_PUMP_IDLE_MAX					0x09   //���������� ����������� ������� ������� ������


//2 ����� ��������� ��������� �������!!!
#define TO_CONTROLE_BY_SHEDULLE		CONTROLE_BY_SHEDULLE   		//��������� ���������� �� ����������
#define TO_CONTROLE_BY_FHOTOSENSOR  CONTROLE_BY_FHOTOSENSOR		//��������� ���������� �� �����������
#define TO_CONTROLE_BY_SERVER       CONTROLE_BY_SERVER   		//��������� ���������� ��������
#define TO_CONTROLE_BY_SERVER_HARD  CONTROLE_BY_SERVER_HARD		//��������� ���������� ��������
#define TO_CONTROLE_BY_ASTR_TIMER 	CONTROLE_BY_ASTR_TIMER		//��������� ���������� �� ���������������� �������
#define UPP_START						     0x0200   //����� ���������
#define UPP_STOP							 0x0100   //��op ���������

#endif

#include "main.h"

#define UNITS_TOTAL		6
#define UNITS_MAXLEN	16

#define ACCESS_LEVEL_NONE		0
#define ACCESS_LEVEL_SERVICE	1
#define ACCESS_LEVEL_MASTER		2
#define ACCESS_LEVEL_MFR		3



extern PUBLIC const char* gc_pacUnits[];
extern PUBLIC tsConfig g_sConfig;
extern PUBLIC uint8_t g_accessLevel;
extern PUBLIC const uint32_t *gc_Unique_ID;

PUBLIC void Global_InitVars(void);

//��������� �������
#define ARCHIVE_SIZE 20
extern PUBLIC tsEvent g_event[ARCHIVE_SIZE];
extern PUBLIC uint8_t g_objectId[16];   // objectId

extern PUBLIC uint16_t g_errorCode;//1���-���� �������, 2���-�������������� ��������� ����� ���������������� �������, 3���-������������� ������� ������ ���������� ����������� � ��������� ������������ ���������, 4���-���������� ������� ���������� � ������ ������
extern PUBLIC uint16_t g_eventCode;
extern PUBLIC uint8_t g_MaxTimeManual;

extern PUBLIC uint8_t g_countTry;   	//���������� ������� �������� ������ � ���� �� ����� ��������� (������). ��� 0 - �� ������������
extern PUBLIC uint8_t g_countTryEvent; //���������� ������� �������� ������ � ���� �� ����� ��������� (�������) , ��� 0 - �� ������������

extern PUBLIC uint8_t g_restart;

extern PUBLIC uint8_t g_errorSendInterval; //�������� ����� ���������� ���������� ����� ��������� ������
extern PUBLIC uint8_t g_isArchive;     //��� g_isArchiv� > 0 - ����� �����
extern PUBLIC uint8_t g_fullNA;
extern PUBLIC uint32_t g_Event_timeMessageSend;
extern PUBLIC uint32_t g_Event_timeMessageReceive;
extern PUBLIC uint32_t g_Error_timeMessageSend;
extern PUBLIC uint32_t g_Error_timeMessageReceive;

extern PUBLIC uint8_t g_stateLight;
extern PUBLIC uint8_t g_state_photosensorIn; 	//1/0- ���������� ��������� ������ �� ���������/����������
extern PUBLIC uint8_t g_state_photosensorOut; 	//1/0- ������/��������� ���������� �� ����������

extern PUBLIC uint8_t g_shedulle;    	// ������� �������� ����������
extern PUBLIC uint8_t g_do_lightAction; // ��������� �������� �� ���������� (���/����)


//sunrise ��������������� ������
extern PUBLIC double g_lon;
extern PUBLIC double g_lat;
extern PUBLIC double g_timeDiff;;
extern PUBLIC double g_sunRise;
extern PUBLIC double g_sunSet;
extern PUBLIC signed char g_after;
extern PUBLIC signed char g_before;

//extern PUBLIC uint16_t g_stateExchange; //���������: ���/����;������ ���������� � �.� . ����� � ligthControle.h

extern PUBLIC double g_RiseTime[5];    //����� ���������� ��������� � ������ ���������� (����� ���� �������������, ���� ���������� �����)
extern PUBLIC double g_SetTime[5]; //����� ��������� ��������� � ������ �������� (����� ���� ������ ��� ����� 24, ���� ����������� �����)


//��� �������
extern PUBLIC uint8_t g_debug;
extern PUBLIC uint16_t g_timeout;

extern PUBLIC uint8_t g_writeConfig;


extern PUBLIC uint8_t g_sControlMode[5];
extern PUBLIC uint32_t g_sOff[5][3];
extern PUBLIC uint32_t g_sOn[5][3];
extern PUBLIC int8_t g_astrLight;

//��������, ������� ������ 04.07.19
extern PUBLIC uint8_t  gPrintf;

extern PUBLIC uint16_t g_errorMaster; // ������ ���, �������� �� �����: 0-�� �� �������, 1-���1 �� �������, 2-���2 �� �������, 3-���3 �� �������
extern PUBLIC uint16_t g_eventMaster; // ������� �� ���� ��������, ���
extern PUBLIC uint8_t g_isFlag1; // ���� ���������� �� ������� TIM3

//g_errorMaster ����� ������ �� ���� �������������
#define WLS_NOT_RESPONDED				0
#define UPP1_NOT_RESPONDED				1
#define UPP2_NOT_RESPONDED				2
#define UPP3_NOT_RESPONDED				3

//g_eventMaster ����� ������� �� ���� �������������
#define ENGINE1_RUN_BUT_MAX				0
#define ENGINE2_RUN_BUT_MAX				1
#define ENGINE3_RUN_BUT_MAX				2
#define ENGINE1_STOP_BUT_MAX			3
#define ENGINE2_STOP_BUT_MAX			4
#define ENGINE3_STOP_BUT_MAX			5

#define LEVEL_RESPONSE_TRY_MAX			3 // ������������ ����� ������� �������� ������� ������
#define PUMP_RESPONSE_TRY_MAX			3 // ������������ ����� ������� �������� ���

///////////////////////��������///////////////////////
extern PUBLIC int8_t g_WLSsetLvl;
extern PUBLIC uint8_t g_MonitoringSet;

extern PUBLIC levelSensor g_WLS; //��������� ������ ������� ������ ����
extern PUBLIC softStarter UPP_LEFT; //��������� ������ ���1
extern PUBLIC softStarter UPP_RIGHT; //��������� ������ ���2

extern PUBLIC softStarter* g_UPPstruct;
extern PUBLIC softStarter* g_SecondUPP;

extern PUBLIC uint8_t g_leftUppActivity;
extern PUBLIC uint8_t g_rightUppActivity;

extern PUBLIC uint32_t gpioPortStatusWork;
extern PUBLIC uint32_t gpioPinStatusWork;

extern PUBLIC uint8_t gTransferAllow; // ���������� �� �������� ������� ����� ������������� � ������
#define CONTROL_BY_AUTO		0
#define CONTROL_BY_SERVER		1
extern PUBLIC uint8_t gControlMode;

//��������, ������� ������ 04.07.19
extern PUBLIC uint16_t g_errorMaster; // ������ ���, �������� �� �����: 0-�� �� �������, 1-���1 �� �������, 2-���2 �� �������, 3-���3 �� �������
extern PUBLIC uint16_t g_eventMaster; // ������� �� ���� ��������, ���
extern PUBLIC uint8_t g_isFlag1; // ���� ���������� �� ������� TIM3

extern PUBLIC rqstStruct req; 			//��������� �������� ������

extern PUBLIC uint8_t nCommand;

extern PUBLIC uint8_t g_isAnswer;
extern PUBLIC uint8_t g_frame[40];
extern PUBLIC uint8_t g_frameLen;
extern PUBLIC uint8_t g_secound;
extern PUBLIC uint8_t gWatchDog;

extern PUBLIC uint8_t gtimeout;


// ������ �������� ��������� ���
extern PUBLIC uint8_t g_forseToError; //���� ���������� �������
extern PUBLIC uint16_t secBeforeOn; //������ �� ��������� ���

extern PUBLIC uint8_t g_WLSserverMode;  // WLS �� �������: 0- �� �����������; 1- ������������, ���� �� ������� ��������
															 //2- ������ �� ������� (��������� ��������� ����/���� ���������
extern PUBLIC uint8_t g_WLSfromserver; 		  //���� WLS �� �������
extern PUBLIC u8 gCountNotAnswerController;
extern PUBLIC u8 gTimerForTalking;
extern PUBLIC u8 gIsRestartMC;
extern PUBLIC u8 gIsAnswerMC;
extern PUBLIC uint8_t g_dataTalk[40];
extern PUBLIC uint8_t g_timeout2;


#define TIME_UPP_ACTIVE						65

#define REQUEST_TIME 					2  //������ ������� ������ ���������, ���
//#define EVENT_HANDLING_TIME 			10  //������ ������� ��������� �������, ���

extern PUBLIC u8 gFromUsart;

#define USART1_NUMBER 					1
#define USART2_NUMBER 					2
#define USART3_NUMBER 					3
#define UART4_NUMBER 					4
#define UART5_NUMBER 					5

extern tsCallback g_tsCb;
#endif



