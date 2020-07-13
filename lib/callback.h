#ifndef __CALLBACK_H__
#define __CALLBACK_H__

#include "main.h"

typedef struct
{
	ACTION_CALLBACK(cb);
	u32 param;
	u8 hdl;
} tsCallback;

#define CALLBACKS_TOTAL			32

PUBLIC void Callback_InitVars(void);
PUBLIC void Callback_Action(void);
PUBLIC void Callback_Save(tsCallback *psCbSave);

#endif
