#ifndef __SLEEP_RESET_H__

	#include "main.h"
	#ifdef __BKP16_H__
		#define __SLEEP_RESET_H__

		#define SLEEP_TYPE_NEVER		(0)
		#define SLEEP_TYPE_WFI			(1)
		#define SLEEP_TYPE_STOP			(2)

		#define SLEEP_FLAG_USART1_RX	(1<<0)
		#define SLEEP_FLAG_SWTIMERS		(1<<1)
		#define SLEEP_FLAG_NO_DS		(1<<2)
		#define SLEEP_FLAG_SYSTMR		(1<<3)
		#define SLEEP_FLAG_RF			(1<<4)
		#define SLEEP_FLAG_HWTIMERS		(1<<5)
		#define SLEEP_FLAG_NO_RESET		(1<<6)
		#define SLEEP_FLAG_ADC			(1<<7)
		#define SLEEP_FLAG_USART1_TX	(1<<8)
		#define SLEEP_FLAG_USART2_RX	(1<<9)
		#define SLEEP_FLAG_USART2_TX	(1<<10)
		#define SLEEP_FLAG_USART3_RX	(1<<11)  //*
		#define SLEEP_FLAG_USART3_TX	(1<<12) //*
		#define SLEEP_FLAG_UART4_TX		(1<<13)  //*
		#define SLEEP_FLAG_UART4_RX		(1<<14) //*
		#define SLEEP_FLAG_UART5_TX		(1<<15) //*
		#define SLEEP_FLAG_UART5_RX		(1<<16) //*

		#define RESET_FLAG_USART1_TX	(1<<0)
		#define RESET_FLAG_USART1_RX	(1<<1)
		#define RESET_FLAG_WFLASH		(1<<2)
		#define RESET_FLAG_USART2_TX	(1<<3)
		#define RESET_FLAG_USART2_RX	(1<<4)
		#define RESET_FLAG_USART3_TX	(1<<5) //*
		#define RESET_FLAG_USART3_RX	(1<<6) //*
		#define RESET_FLAG_UART4_TX		(1<<7)
		#define RESET_FLAG_UART4_RX		(1<<8)
		#define RESET_FLAG_UART5_TX		(1<<9) //*
		#define RESET_FLAG_UART5_RX		(1<<10) //*

		PUBLIC void SleepReset_Init(uint8_t sleepType, uint32_t sleepFlags, uint32_t resetFlags);
		PUBLIC void Reset_Try(void);
		PUBLIC void Sleep_Try(void);
		PUBLIC void Sleep_Config(uint32_t __flags, uint8_t newState);
		PUBLIC void Reset_Config(uint32_t __flags, uint8_t newState);
		PUBLIC void Reset_Soft(void);

	#endif
#endif
