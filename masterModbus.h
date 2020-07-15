#include "stdafx.h"
#include "main.h"

#define GPIO_PORT_UPP_SELECT					GPIOB
#define GPIO_PIN_UPP_SELECT   					GPIO_Pin_6

#define IS_LEFT_UPP_ACTIVE						1-GPIO_ReadInputDataBit(GPIO_PORT_UPP_SELECT, GPIO_PIN_UPP_SELECT)
#define IS_RIGHT_UPP_ACTIVE						GPIO_ReadInputDataBit(GPIO_PORT_UPP_SELECT, GPIO_PIN_UPP_SELECT)

#define GPIO_PORT_LEFT_STATUS_WORK  			GPIOB			//PB7
#define GPIO_PIN_LEFT_STATUS_WORK				GPIO_Pin_5
#define GPIO_PORT_RIGHT_STATUS_WORK				GPIOB			//PB8
#define GPIO_PIN_RIGHT_STATUS_WORK  			GPIO_Pin_7

/*
#define GPIO_PORT_LEFT_ACTIVITY	   				GPIOB			//PB6
#define GPIO_PIN_LEFT_ACTIVITY      			GPIO_Pin_6
#define GPIO_PORT_RIGHT_ACTIVITY			   	GPIOB			//PB5
#define GPIO_PIN_RIGHT_ACTIVITY		        	GPIO_Pin_5
// Правильное
#define GPIO_PORT_LEFT_STATUS_WORK  			GPIOB			//PB7
#define GPIO_PIN_LEFT_STATUS_WORK				GPIO_Pin_7
#define GPIO_PORT_RIGHT_STATUS_WORK				GPIOB			//PB8
#define GPIO_PIN_RIGHT_STATUS_WORK  			GPIO_Pin_8
*/

//Для отладки
/*
#define GPIO_PORT_LEFT_STATUS_WORK  			GPIOB			//PB7
#define GPIO_PIN_LEFT_STATUS_WORK				GPIO_Pin_8
#define GPIO_PORT_RIGHT_STATUS_WORK				GPIOB			//PB8
#define GPIO_PIN_RIGHT_STATUS_WORK  			GPIO_Pin_7
*/

#define GPIO_PORT_STOP     						GPIOB			//PB0
#define GPIO_PIN_STOP      						GPIO_Pin_0
#define GPIO_PORT_START    						GPIOB			//PB1
#define GPIO_PIN_START      					GPIO_Pin_1

#define GPIO_PORT_START_CONTACTOR     			GPIOB				//PB1
#define GPIO_PIN_START_CONTACTOR     			GPIO_Pin_0
#define GPIO_PORT_STOP_CONTACTOR     			GPIOA				//PB15
#define GPIO_PIN_STOP_CONTACTOR      			GPIO_Pin_7

#define GPIO_PORT_RESTART_OTHER_MC 				GPIOA  			//PA7
#define GPIO_PIN_RESTART_OTHER_MC     			GPIO_Pin_7

#define DELAY50000 volatile int i;for(i=0; i<500000; i++);
#define TIME_UNDEFINED 							0xFFFFFFFF
#define TIME_RELEASE 								 1573310265    //дата Sat, 09 Nov 2019 14:37:45

#define IS_LEFT_UPP_ACTIVE						GPIO_ReadInputDataBit(GPIO_PORT_LEFT_ACTIVITY, GPIO_PIN_LEFT_ACTIVITY)
#define IS_RIGHT_UPP_ACTIVE						GPIO_ReadInputDataBit(GPIO_PORT_RIGHT_ACTIVITY, GPIO_PIN_RIGHT_ACTIVITY)
extern PUBLIC uint8_t g_leftUppActivity;
extern PUBLIC uint8_t g_rightUppActivity;

#define UPP_CONTACTOR_OFF						0
#define UPP_CONTACTOR_ON						1

void master_init(void);
void masterGpioInit();
void masterRequest(u8 hdl, u32 time);
void monitoring(u8 hdl, u32 param);
void TalkingBtwnCtrs();
uint8_t MasterModbusSend(uint8_t NA, uint8_t fun, uint8_t* data, uint8_t len, int8_t lengthWait, uint8_t tryes, uint8_t timeout);
PUBLIC uint8_t MasterModbusSendUsart2(uint8_t NA, uint8_t fun, uint8_t* data, uint8_t len, int8_t lengthWait, uint8_t tryes, uint8_t timeout);
uint8_t WlsStatusRqst();
PUBLIC uint8_t myWLSstatusRqst();
uint8_t contactorSetOn();
uint8_t contactorSetOff();
uint8_t contactorStatus();
void controlUnitRqst();
void reqUPPTo_CONTROL_BY_SERVER(void);
void led5blink();
void wait_(uint8_t timeout);
void equipment_config(void);
void WLSparseHeight(uint8_t* data);
uint8_t isMaxPumpIdleTime(uint8_t wlsOk);
uint8_t isMinPumpTimeAfterStartUpp();
uint8_t contactorSetOffTry();
u8 CalcCRC8FromChId();

void UPPchoice();
void UPPstartTry(softStarter* Upp);
uint8_t UPPstop(softStarter* Upp);
uint8_t UPPautoModeSetOffByControlWordNew(softStarter* Upp);
uint8_t UPPautoModeSetOn(softStarter* Upp);
uint8_t UPPfaultReset(softStarter* Upp);
uint8_t UPPautoModeSetOff(softStarter* Upp);
uint8_t UPPstopRqst(softStarter* Upp);
uint8_t UPPstartRqst(softStarter* Upp);
uint8_t UPPstatusRqst(softStarter* Upp);
void WriteToBKPLastStopTime(softStarter* Upp);
void WriteToBKPLastStartTime(softStarter* Upp);
void ReadFromBKP();
void UPP_Start(u8 hdl, u32 param);
void UPPstartCallback(u8 hdl, u32 param);
void UPPstartNew(softStarter* Upp);

#define BKP_FIRST_LAST_START1   			BKP_DR1
#define BKP_FIRST_LAST_START2				BKP_DR2
#define BKP_FIRST_LAST_STOP1				BKP_DR3
#define BKP_FIRST_LAST_STOP2				BKP_DR4
#define BKP_SECOND_LAST_START1				BKP_DR5
#define BKP_SECOND_LAST_START2				BKP_DR6
#define BKP_SECOND_LAST_STOP1				BKP_DR7
#define BKP_SECOND_LAST_STOP2				BKP_DR8
//#define BKP_CRTL_MODE						BKP_DR11
