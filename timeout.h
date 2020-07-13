/*
  timeout.h
*/

#include "common.h"

#ifndef _TIMEOUT_H_
#define _TIMEOUT_H_

//#define NUMTIMEOUTS 8

typedef HANDLER HTIMEOUT;

//typedef void (*TIMEOUTFUNC)(HTIMEOUT,void *pParam);

void initTimeOuts(void);

HTIMEOUT  createTimeOut(SIGNAL_CB pFunc);
void launchTimeOut(HTIMEOUT hTO,uint16_t WaitTime,void *pParam,uint32_t uParam);
bool isTimeOutLaunched(HTIMEOUT hTO);
bool abortTimeOut(HTIMEOUT hTO);
bool destroyTimeOut(HTIMEOUT hTO);

#endif
