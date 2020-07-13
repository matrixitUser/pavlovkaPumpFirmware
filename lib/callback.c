#include "main.h"

#ifdef __CALLBACK_H__

PRIVATE tsCallback _asCallback[CALLBACKS_TOTAL];

PUBLIC void Callback_InitVars(void)
{
	u8 i;
	for(i=0; i<CALLBACKS_TOTAL; i++)
	{
		_asCallback[i].cb = NULL;
	}
}

PUBLIC void Callback_Action(void)
{
	u8 i;
	for(i=0; i<CALLBACKS_TOTAL; i++)
	{
		tsCallback *psCb = &_asCallback[i];
		if(psCb->cb != NULL)
		{
			psCb->cb(psCb->hdl, psCb->param);
			psCb->cb = NULL;
		}
	}
}

PUBLIC void Callback_Save(tsCallback *psCbSave)
{
	u8 i;
	for(i=0; i<CALLBACKS_TOTAL; i++) {
		tsCallback *psCb = &_asCallback[i];
		if(psCb->cb == NULL)
		{
			psCb->cb = psCbSave->cb;	//сохранение кб
			psCb->hdl = psCbSave->hdl;
			psCb->param = psCbSave->param;
			break;
		}
	}
}

#endif
