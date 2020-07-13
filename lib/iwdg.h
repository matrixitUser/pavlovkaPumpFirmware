#ifndef __IWDG_H__
#define __IWDG_H__

#include "main.h"

#define IWDG_KICK_INTERVAL			INTERVAL(0,0,20)
#define IWDG_KICK_INTERVAL_MSEC		INTERVAL_MSEC(0,0,(IWDG_KICK_INTERVAL),0)

void IWDG_Init(void);

#endif
