#ifndef __SWALARM_H__

	#include "main.h"
	#if defined(__RTCALARM_H__)

		#define __SWALARM_H__


		#define SWALARM_DIV_FACTOR		1

		#define SWALARM_OPTIMIZE_TIME	SWALARM_OPTIMIZE_TIME
		#define SWALARM_DIV(x)			((x)>>SWALARM_DIV_FACTOR)
		#define SWALARM_UNDIV(x)		((x)<<SWALARM_DIV_FACTOR)
		#define SWALARM_MIN_DELAY		SWALARM_UNDIV(1)

		typedef struct
		{
			u32 expire_div;				//общее время звонка будильника

			u16 interval_div;			//период для интервального таймера (для расчёта expire_div)
			u16 offset_div;				//смещение периода интервального таймера (для расчёта expire_div)

			tsCallback sCb;

			u8 handler;
			u8 state;					//статус: обычный, интервальный или выключен
		}
		SWAlarm;

		#define SWALARM_STATE_OFF				0x00
		#define SWALARM_STATE_ON				0x01
		#define SWALARM_STATE_INTERVAL			0x02
		#define SWALARM_STATE_INTERVAL_ONCE		0x03
		#define _SWALARM_STATE_TRANSITION		0x80

		#define SWALARM_TOTAL			6

		//PUBLIC void SWAlarm_Handler(void);

		PUBLIC u8 SWAlarm_Add(u16 delay_sec, ACTION_CALLBACK(callback));
		//PUBLIC u8 SWAlarm_AddInterval(u32 interval_sec, u16 offset_sec, ACTION_CALLBACK(callback));

		PUBLIC void SWAlarm_InitVars(void);
		PUBLIC void SWAlarm_Tick(void);
		PUBLIC u32 SWAlarm_Next(u32 time);

	#endif

#endif
