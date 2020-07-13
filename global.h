#ifndef __GLOBAL_H__
#define __GLOBAL_H__

#define MONITORING_TIME 		15     //период времени мониторинга состояния, сек

#define ERR_TIMER				0x00   //Cброшен или не установлен часовой таймера
#define ERR_OUT_MCU_AND_REAL	0x02   //Несоответсвие сигнала выхода управления контактором и состояния включенности освещения
#define ERR_HANDMODE_TIMEOUT	0x03   //Превышение времени нахождения в ручном режиме
#define ERR_RESTART				0x04   //Перезагрузка
#define SIGNALING				0x10   //Сработка сигнализации

#ifdef LIGHT_CONTROLE
//2 байт состояния (ошибок)
#define ERR_ASTR_TIMER			0x01   //Несоответстиве состояния включенности освещения астрономическому таймеру,

//2 байта изменения состояния СОБЫТИЯ!!!
#define TO_CONTROLE_BY_SHEDULLE		CONTROLE_BY_SHEDULLE   		//Установка управления по расписанию
#define TO_CONTROLE_BY_FHOTOSENSOR  CONTROLE_BY_FHOTOSENSOR		//Установка управления по фотодатчику
#define TO_CONTROLE_BY_SERVER       CONTROLE_BY_SERVER   		//Установка управления сервером
#define TO_CONTROLE_BY_SERVER_HARD  CONTROLE_BY_SERVER_HARD		//Установка управления сервером
#define TO_CONTROLE_BY_ASTR_TIMER 	CONTROLE_BY_ASTR_TIMER		//Установка управления по астрономическому таймеру
#define TO_LIGTH_ON				     0x0100   //Включение освещения
#define TO_LIGTH_OFF				 0x0200   //Выключение освещения

#endif

#ifdef PAVLOVKA
//2 байт состояния (ошибок)
#define ERR_NOT_ACTIVITY_UPP				0x01   //Cброшен или не установлен часовой таймера
#define ERR_UPP_NO_START					0x02   //Не удалось запустить двигатель
#define ERR_UPP_NO_STOP						0x03   //Не удалось остановит двигатель

#define ERR_WLS_NOT_RESPONCE				0x05   //Нет ответа от WLS(башни)
#define ERR_UPP_NOT_RESPONCE				0x06   //Нет ответа от UPP
#define ERR_WLS_SENSOR						0x07   //Ошибка датчика уровня (нет минимума)
#define ERR_UPP_AUTOMODE_SET				0x08   //Не удалось перевести УПП в автомод
#define ERR_PUMP_IDLE_MAX					0x09   //Превышение допустимого времени простоя насоса


//2 байта изменения состояния СОБЫТИЯ!!!
#define TO_CONTROLE_BY_SHEDULLE		CONTROLE_BY_SHEDULLE   		//Установка управления по расписанию
#define TO_CONTROLE_BY_FHOTOSENSOR  CONTROLE_BY_FHOTOSENSOR		//Установка управления по фотодатчику
#define TO_CONTROLE_BY_SERVER       CONTROLE_BY_SERVER   		//Установка управления сервером
#define TO_CONTROLE_BY_SERVER_HARD  CONTROLE_BY_SERVER_HARD		//Установка управления сервером
#define TO_CONTROLE_BY_ASTR_TIMER 	CONTROLE_BY_ASTR_TIMER		//Установка управления по астрономическому таймеру
#define UPP_START						     0x0200   //Старт двигателя
#define UPP_STOP							 0x0100   //Стop двигателя

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

//Структура событий
#define ARCHIVE_SIZE 20
extern PUBLIC tsEvent g_event[ARCHIVE_SIZE];
extern PUBLIC uint8_t g_objectId[16];   // objectId

extern PUBLIC uint16_t g_errorCode;//1бит-сбой времени, 2бит-Несоответстиве состояния света астрономическому таймеру, 3бит-Несоответсвие сигнала выхода управления контактором и состояния включенности освещения, 4бит-Превышение времени нахождения в ручном режиме
extern PUBLIC uint16_t g_eventCode;
extern PUBLIC uint8_t g_MaxTimeManual;

extern PUBLIC uint8_t g_countTry;   	//Количество попыток отправки одного и того же слова состояния (ошибок). при 0 - не отправляется
extern PUBLIC uint8_t g_countTryEvent; //Количество попыток отправки одного и того же слова состояния (событий) , при 0 - не отправляется

extern PUBLIC uint8_t g_restart;

extern PUBLIC uint8_t g_errorSendInterval; //интервал между повторными отправками слова состояния ошибок
extern PUBLIC uint8_t g_isArchive;     //при g_isArchivе > 0 - вести архив
extern PUBLIC uint8_t g_fullNA;
extern PUBLIC uint32_t g_Event_timeMessageSend;
extern PUBLIC uint32_t g_Event_timeMessageReceive;
extern PUBLIC uint32_t g_Error_timeMessageSend;
extern PUBLIC uint32_t g_Error_timeMessageReceive;

extern PUBLIC uint8_t g_stateLight;
extern PUBLIC uint8_t g_state_photosensorIn; 	//1/0- фотодатчик установил сигнал на включение/выключение
extern PUBLIC uint8_t g_state_photosensorOut; 	//1/0- Подано/выключено напряжение на фотодатчик

extern PUBLIC uint8_t g_shedulle;    	// регистр сработки расписания
extern PUBLIC uint8_t g_do_lightAction; // возможное действие по расписание (вкл/выкл)


//sunrise Астрономический таймер
extern PUBLIC double g_lon;
extern PUBLIC double g_lat;
extern PUBLIC double g_timeDiff;;
extern PUBLIC double g_sunRise;
extern PUBLIC double g_sunSet;
extern PUBLIC signed char g_after;
extern PUBLIC signed char g_before;

//extern PUBLIC uint16_t g_stateExchange; //Изменения: Вкл/выкл;Режимы управления и т.д . Флаги в ligthControle.h

extern PUBLIC double g_RiseTime[5];    //Время выключения освещения с учетом опережения (может быть отрицательным, если предыдущие сутки)
extern PUBLIC double g_SetTime[5]; //Время включения освещения с учетом задержки (может быть больше или равно 24, если последующие сутки)


//Для отладки
extern PUBLIC uint8_t g_debug;
extern PUBLIC uint16_t g_timeout;

extern PUBLIC uint8_t g_writeConfig;


extern PUBLIC uint8_t g_sControlMode[5];
extern PUBLIC uint32_t g_sOff[5][3];
extern PUBLIC uint32_t g_sOn[5][3];
extern PUBLIC int8_t g_astrLight;

//Павловка, водяные насосы 04.07.19
extern PUBLIC uint8_t  gPrintf;

extern PUBLIC uint16_t g_errorMaster; // Ошибки УПП, датчиков по битам: 0-ДУ не ответил, 1-УПП1 не ответил, 2-УПП2 не ответил, 3-УПП3 не ответил
extern PUBLIC uint16_t g_eventMaster; // События по всем датчикам, УПП
extern PUBLIC uint8_t g_isFlag1; // Флаг прерывания по таймеру TIM3

//g_errorMaster общие ошибки по всем оборудованиям
#define WLS_NOT_RESPONDED				0
#define UPP1_NOT_RESPONDED				1
#define UPP2_NOT_RESPONDED				2
#define UPP3_NOT_RESPONDED				3

//g_eventMaster общие события по всем оборудованиям
#define ENGINE1_RUN_BUT_MAX				0
#define ENGINE2_RUN_BUT_MAX				1
#define ENGINE3_RUN_BUT_MAX				2
#define ENGINE1_STOP_BUT_MAX			3
#define ENGINE2_STOP_BUT_MAX			4
#define ENGINE3_STOP_BUT_MAX			5

#define LEVEL_RESPONSE_TRY_MAX			3 // максимальное число попыток запросов датичка уровня
#define PUMP_RESPONSE_TRY_MAX			3 // максимальное число попыток запросов УПП

///////////////////////ПАВЛОВКА///////////////////////
extern PUBLIC int8_t g_WLSsetLvl;
extern PUBLIC uint8_t g_MonitoringSet;

extern PUBLIC levelSensor g_WLS; //Структура данных датчика уровня воды
extern PUBLIC softStarter UPP_LEFT; //Структура данных УПП1
extern PUBLIC softStarter UPP_RIGHT; //Структура данных УПП2

extern PUBLIC softStarter* g_UPPstruct;
extern PUBLIC softStarter* g_SecondUPP;

extern PUBLIC uint8_t g_leftUppActivity;
extern PUBLIC uint8_t g_rightUppActivity;

extern PUBLIC uint32_t gpioPortStatusWork;
extern PUBLIC uint32_t gpioPinStatusWork;

extern PUBLIC uint8_t gTransferAllow; // Разрешение на трансфер пакетов между Оборудованием и верхом
#define CONTROL_BY_AUTO		0
#define CONTROL_BY_SERVER		1
extern PUBLIC uint8_t gControlMode;

//Павловка, водяные насосы 04.07.19
extern PUBLIC uint16_t g_errorMaster; // Ошибки УПП, датчиков по битам: 0-ДУ не ответил, 1-УПП1 не ответил, 2-УПП2 не ответил, 3-УПП3 не ответил
extern PUBLIC uint16_t g_eventMaster; // События по всем датчикам, УПП
extern PUBLIC uint8_t g_isFlag1; // Флаг прерывания по таймеру TIM3

extern PUBLIC rqstStruct req; 			//Структура текущего опроса

extern PUBLIC uint8_t nCommand;

extern PUBLIC uint8_t g_isAnswer;
extern PUBLIC uint8_t g_frame[40];
extern PUBLIC uint8_t g_frameLen;
extern PUBLIC uint8_t g_secound;
extern PUBLIC uint8_t gWatchDog;

extern PUBLIC uint8_t gtimeout;


// Таймер задержки включения УПП
extern PUBLIC uint8_t g_forseToError; //флаг активности таймера
extern PUBLIC uint16_t secBeforeOn; //Секунд до включения УПП

extern PUBLIC uint8_t g_WLSserverMode;  // WLS из сервера: 0- не использутся; 1- используется, если не удалось опросить
															 //2- только из сервера (позволяет управлять стоп/пуск двигателя
extern PUBLIC uint8_t g_WLSfromserver; 		  //байт WLS из сервера
extern PUBLIC u8 gCountNotAnswerController;
extern PUBLIC u8 gTimerForTalking;
extern PUBLIC u8 gIsRestartMC;
extern PUBLIC u8 gIsAnswerMC;
extern PUBLIC uint8_t g_dataTalk[40];
extern PUBLIC uint8_t g_timeout2;


#define TIME_UPP_ACTIVE						65

#define REQUEST_TIME 					2  //период времени опроса устройств, сек
//#define EVENT_HANDLING_TIME 			10  //период времени обработки событий, сек

extern PUBLIC u8 gFromUsart;

#define USART1_NUMBER 					1
#define USART2_NUMBER 					2
#define USART3_NUMBER 					3
#define UART4_NUMBER 					4
#define UART5_NUMBER 					5

extern tsCallback g_tsCb;
#endif



