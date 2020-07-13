#include "main.h"
#ifdef __COUNTER_H__
	#ifndef __GPIOCTRL_H__
		#define __GPIOCTRL_H__
		PUBLIC uint8_t GpioCtrl_TmrRtcCheck_GetDTime(void);
		PUBLIC void GpioCtrl_TmrRtcCheck_Init(void);

		PUBLIC void GpioCtrl_TmrFilter_Init(uint16_t tmrPeriod_msm1);
		PUBLIC uint32_t GpioCtrl_GetPins(void);

		PUBLIC void GpioCtrl_TmrStatusLed_Init(void);

		PUBLIC void GpioCtrl_InitDefaults(void);
		PUBLIC void GpioCtrl_InitLeds(void);

	#endif
#endif
