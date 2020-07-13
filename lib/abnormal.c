#include "main.h"

#ifdef __ABNORMAL_H__

PRIVATE tsArchive abnormal;

PRIVATE void init(void)
{
#if (FLAGS & FLAGS_ENABLE_DEBUG) > 0
	uint8_t i;
	for(i=0; i<10; i++)
	{
		abnormal.name[i] = 0;
	}
	strcpy((char *)&abnormal.name, "abnormal");
#endif
	abnormal.pages = ABNORMAL_PAGES;
	abnormal.record_size = sizeof(tsAbnormalEvent);
	abnormal.start_page_addr = ABNORMAL_PAGE;
}

PUBLIC void Abnormal_Init(uint8_t configIsOk)
{
	init();
	if(configIsOk)
	{
		Archive_Init(&abnormal);
	}
	else
	{
		Archive_Reset(&abnormal);
	}
}

PUBLIC void Abnormal_Write(tsAbnormalEvent* precord)
{
	Archive_Write(&abnormal, precord);
}

PUBLIC uint8_t Abnormal_GetPage(void)
{
	return Archive_GetPage(&abnormal);
}

PUBLIC uint8_t Abnormal_GetPageIndex(void)
{
	return Archive_GetPageIndex(&abnormal);
}

PUBLIC ErrorStatus Abnormal_GetByPageOffset(tsAbnormalEvent *pRecord, uint8_t page, uint8_t record)
{
	return Archive_Get(pRecord, sizeof(tsAbnormalEvent), &abnormal, page, record);
}

PUBLIC ErrorStatus Abnormal_GetByOffset(tsAbnormalEvent *pRecord, uint16_t offset)
{
	return Archive_GetOffset(pRecord, sizeof(tsAbnormalEvent), &abnormal, offset);
}

PUBLIC ErrorStatus Abnormal_GetLastByType(tsAbnormalEvent *pRecord, uint32_t type)
{
	type &= AEID_TYPE_MASK;
	uint16_t offset;
	for(offset = 0; Archive_GetOffset(pRecord, sizeof(tsAbnormalEvent), &abnormal, offset) == SUCCESS; offset++)
	{
		if((pRecord->u32AeId & AEID_TYPE_MASK) == type) return SUCCESS;
	}
	return ERROR;
}

////

PUBLIC tsAbnormalEvent Abnormal_MakeRecord(uint32_t time, uint32_t aeid)
{
	tsAbnormalEvent record;
	record.u32Ts = time;
	record.u32AeId = aeid;
	return record;
}

PUBLIC uint16_t Abnormal_GetResetFlags(void)
{
	uint16_t flags = 0x0000;

	if(RCC_GetFlagStatus(RCC_FLAG_PINRST) == SET)//Pin reset
	{
		flags |= AEID_RESTART_PINRST;
	}

	if(RCC_GetFlagStatus(RCC_FLAG_PORRST) == SET)//POR/PDR reset
	{
		flags |= AEID_RESTART_PORRST;
	}

	if(RCC_GetFlagStatus(RCC_FLAG_SFTRST) == SET)//Software reset
	{
		flags |= AEID_RESTART_SFTRST;
	}

	if(RCC_GetFlagStatus(RCC_FLAG_IWDGRST) == SET)//Independent Watchdog reset
	{
		flags |= AEID_RESTART_IWDGRST;
	}

	if(RCC_GetFlagStatus(RCC_FLAG_WWDGRST) == SET)//Window Watchdog reset
	{
		flags |= AEID_RESTART_WWDGRST;
	}

	if(RCC_GetFlagStatus(RCC_FLAG_LPWRRST) == SET)//Low Power reset
	{
		flags |= AEID_RESTART_LPWRRST;
	}

	RCC_ClearFlag();

	return flags;
}

#endif
