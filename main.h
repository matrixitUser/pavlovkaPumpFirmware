#ifndef __MAIN_H__
#define __MAIN_H__

#define PAVLOVKA
//#define LIGHT_CONTROLE
#ifdef LIGHT_CONTROLE
///////// LIGHT  ////////////////////
//регистр расписания (при срабатывании расписания и нахождения управления не в режиме  CONTROLE_BY_SHEDULLE
#define SHEDULLE_IS_NOT_ACTIVE  (0)
#define SHEDULLE_IS_ACTIVE      (1)

//Метод управления вкл/выкл освещения
#define CONTROLE_BY_SHEDULLE    	(0x00)
#define CONTROLE_BY_FHOTOSENSOR 	(0x01)
#define CONTROLE_BY_SERVER      	(0x02)  //ручной режим ..
#define CONTROLE_BY_SERVER_HARD 	(0x12)  //ручной режим ...в чем разница?
#define CONTROLE_BY_ASTR_TIMER  	(0x03)
#define CONTROLE_BY_ASTR_SHEDULLE   (0x10)  //астрон. астр. таймер + расписание

//Вход на контроллер от контаков после контактора
#define GPIO_PORT_CONTACTOR_IN	   	GPIOB
#define GPIO_PIN_CONTACTOR_IN      	GPIO_Pin_6


#define GPIO_PORT_CONTACTOR_IN1	   	GPIOB			//PB6
#define GPIO_PIN_CONTACTOR_IN1      GPIO_Pin_6
#define GPIO_PORT_CONTACTOR_IN2	   	GPIOB			//PB5
#define GPIO_PIN_CONTACTOR_IN2      GPIO_Pin_5
#define GPIO_PORT_CONTACTOR_IN3	   	GPIOB			//PB7
#define GPIO_PIN_CONTACTOR_IN3      GPIO_Pin_7
#define GPIO_PORT_CONTACTOR_IN4	   	GPIOB			//PB8
#define GPIO_PIN_CONTACTOR_IN4      GPIO_Pin_8

#define GPIO_PORT_12V_IN5	   		GPIOB			//PB3
#define GPIO_PIN_12V_IN5      		GPIO_Pin_3
#define GPIO_PORT_12V_IN6	   		GPIOA			//PA15
#define GPIO_PIN_12V_IN6     	 	GPIO_Pin_15

//out Порт управления освещением
#define GPIO_PORT_LIGHT1     GPIOA  			//PA7
#define GPIO_PIN_LIGHT1      GPIO_Pin_7
#define GPIO_PORT_LIGHT2     GPIOB				//PB0
#define GPIO_PIN_LIGHT2      GPIO_Pin_0
#define GPIO_PORT_LIGHT3     GPIOB				//PB1
#define GPIO_PIN_LIGHT3      GPIO_Pin_1
#define GPIO_PORT_LIGHT4     GPIOB				//PB15
#define GPIO_PIN_LIGHT4      GPIO_Pin_15

#define GPIO_PORT_12V_OUT5	   		GPIOB			//PB4
#define GPIO_PIN_12V_OUT5      		GPIO_Pin_4
#define GPIO_PORT_12V_OUT6	   		GPIOB			//PB9
#define GPIO_PIN_12V_OUT6     	 	GPIO_Pin_9

#define GPIO_PORT_LIGHT     GPIO_PORT_LIGHT1
#define GPIO_PIN_LIGHT      GPIO_PIN_LIGHT1

#define DO_LIGHT_NOT_ACTIVE  (0)
#define DO_LIGHT_IS_ACTIVE   (1)

#define BKP_REGISTER_STATE_LIGHT     (1)  //регистр состояния включенности освещения
#define BKP_REGISTER_STATE_PHOTOSENSOR     (2) //регистр состояния подачи напряжения на фотодатчик

//Вход от фотодатчика
//out Порт PHOTOSENSOR  включение /выключение питания фотодатчика
#define GPIO_PORT_PHOTOSENSOR_OUT   	GPIOB
#define GPIO_PIN_PHOTOSENSOR_OUT      	GPIO_Pin_13

//in Порт FHOTOSENSOR используется первый вход контактора
#define GPIO_PORT_PHOTOSENSOR_IN	   	GPIO_PORT_CONTACTOR_IN1
#define GPIO_PIN_PHOTOSENSOR_IN      	GPIO_PIN_CONTACTOR_IN1

//
#define GPIO_PORT_SIGNALING	   	GPIOA		//PA15
#define GPIO_PIN_SIGNALING     	GPIO_Pin_15


#define PHOTOSENSOR_ENCHANGE_COUNT      0 //число одинакового состояния сигнала от фотодатчика после его изменения

#define DO_PHOTOSENSOR_NOT_ACTIVE  (0)
#define DO_PHOTOSENSOR_IS_ACTIVE   (1)
#endif
//////////////////////////////////////////////
// конфигурация страниц флэш с данными //
//////////////////////////////////////////////
#define START_PAGE_NUMBER		50
#define CONFIG_PAGES			1	//50
#define VBKP_PAGES				4	//51-54
#define ABNORMAL_PAGES			8	//55-62
#define HOURLY_PAGES			65	//63-127
#define PASSWORD_LENGTH			24
#define PASSWORD_MASTER			"X7]JSAm5pxKoiLMzMOr0C$5K"
#define PASSWORD_SERVICE		"matrixit666 service"
//////////////////////////////////////////////

// стандартные драйвера периферии
#include <string.h>
#include "stm32f10x_adc.h"
#include "stm32f10x_bkp.h"
#include "stm32f10x_dbgmcu.h"
#include "stm32f10x_dma.h"
#include "stm32f10x_exti.h"
#include "stm32f10x_flash.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_iwdg.h"
#include "stm32f10x_pwr.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_rtc.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_usart.h"
#include "misc.h"

// инклуд-файлы
#include "device_types.h"
#include "stdafx.h"
#include "hardware.h"
#include "struct.h"
#include "irq_v2.h"
#include "common.h"

#define TYPE 					TYPE_PAVLOVKA
#define VERSION					(1) //14.1- хвс,гвс,магниты; 14.2- элво по 4; 14.3- элво по ; 14.6- Освещение версии 1; 6,2 -Освещение версии 2//* Было (1)
#define NETWORK_ADDRESS			(241)

#define CONFIG_PAGE_NUMBER		(START_PAGE_NUMBER)
#define VBKP_PAGE_NUMBER		(CONFIG_PAGE_NUMBER + 	CONFIG_PAGES)
#define ABNORMAL_PAGE_NUMBER	(VBKP_PAGE_NUMBER + 	VBKP_PAGES)
#define HOURLY_PAGE_NUMBER		(ABNORMAL_PAGE_NUMBER + ABNORMAL_PAGES)
#define FLASH_END_PAGE_NUMBER	(HOURLY_PAGE_NUMBER + 	HOURLY_PAGES)
#define TOTAL_PAGES				(FLASH_END_PAGE_NUMBER - START_PAGE_NUMBER)

#define MIN_PAGE				(HW_FLASH_POINTER+HW_FLASH_SIZE*(START_PAGE_NUMBER))
#define CONFIG_PAGE				(HW_FLASH_POINTER+HW_FLASH_SIZE*(CONFIG_PAGE_NUMBER))
#define VBKP_PAGE				(HW_FLASH_POINTER+HW_FLASH_SIZE*(VBKP_PAGE_NUMBER))
#define ABNORMAL_PAGE			(HW_FLASH_POINTER+HW_FLASH_SIZE*(ABNORMAL_PAGE_NUMBER))
#define HOURLY_PAGE				(HW_FLASH_POINTER+HW_FLASH_SIZE*(HOURLY_PAGE_NUMBER))
#define FLASH_END_PAGE			(HW_FLASH_POINTER+HW_FLASH_SIZE*(FLASH_END_PAGE_NUMBER))

#define ABNORMAL_PAGE_RECORDS	(( HW_FLASH_SIZE - 4 ) / sizeof(tsAbnormalEvent))
#define HOURLY_PAGE_RECORDS		(( HW_FLASH_SIZE - 4 ) / sizeof(tsArchiveRecordPack))

// библиотеки
#include "crc.h"
#include "iwdg.h"
#include "bkp16.h"
#include "sleep_reset.h"

#include "callback.h"
#include "rtcalarm.h"
#include "swalarm_v2.h"

#include "flash.h"
#include "vbkp.h"
#include "counter.h"
#include "archive.h"
#include "hourly.h"
#include "abnormal.h"

#include "usart_v3.h"
#include "usart1.h"
#include "usart2.h"
#include "usart3.h"
#include "uart4.h"
#include "uart5.h"

// сборки
#include "global.h"
#include "config.h"
#include "frame_modbus.h"
#include "frame_transfer.h"
#include "masterModbus.h"
#include "clock.h"
#include "gpioctrl.h"
#include "sigpwm.h"
#include "adc1.h"
#include "time.h"
#include "answerFrameModbus.h"
#include "answerModbusUsart2.h"
#include "talkingBetweenControllers.h"
//#include "sun.h"

#endif
