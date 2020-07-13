#include "main.h"
#if defined(__BKP16_H__)
	#ifndef __RTCALARM_H__
		#define __RTCALARM_H__
		#include "swalarm_v2.h"

		#ifndef RTCALARM_IRQ
		#error RTC Alarm IRQ is UNDEFINED!
		#endif


		#if 0
		#define RTC_INIT_LSI	0x01
		#define RTC_INIT_FIRST	0x02

		PUBLIC uint8_t Rtc_Init(uint8_t u8fInit);
		#else
		PUBLIC void Rtc_Init(void);
		#endif
		u8 RtcAlarm_Reset(void);
		void RtcAlarm_Handler(void);
		void Rtc_SetTime(u32 newtime);
	#endif
#endif
