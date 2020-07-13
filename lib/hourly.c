#include "main.h"

#ifdef __HOURLY_H__

extern PUBLIC uint32_t _curPortDIO;

PRIVATE tsArchive hourly;

PRIVATE void init(void)
{
#if (FLAGS & FLAGS_ENABLE_DEBUG) > 0
	uint8_t i;
	for(i=0; i<10; i++)
	{
		hourly.name[i] = 0;
	}
	strcpy((char *)&hourly.name, "hourly");
#endif
	hourly.pages = HOURLY_PAGES;
	hourly.record_size = sizeof(tsArchiveRecordPack);
	hourly.start_page_addr = HOURLY_PAGE;
}

PRIVATE void _unpackRecord(tsArchiveRecordUnpack *pUnpackRecordTo, tsArchiveRecordPack *pPackRecordFrom)
{
	uint8_t i;

	pUnpackRecordTo->u32Ts = pPackRecordFrom->u32Ts;
	for(i = 0; i < COUNTERS; i++)
	{
		uint32_t u32StatCounter = pPackRecordFrom->au32Counter[i];
		if((u32StatCounter & ARCHIVERECORD_COUNTER_ENABLEMASK) && (g_sConfig.au32CntReleaseTs[i] <= pPackRecordFrom->u32Ts))
		{
			SETBITS(u32StatCounter, ARCHIVERECORD_COUNTER_ENABLEMASK);
		}
		else
		{
			CLRBITS(u32StatCounter, ARCHIVERECORD_COUNTER_ENABLEMASK);
		}
		pUnpackRecordTo->asCounter[i].u8Stat_u24Counter = u32StatCounter;
		pUnpackRecordTo->asCounter[i].i8Point = g_sConfig.asChannel[i].i8Point;
		pUnpackRecordTo->asCounter[i].u8Unit = g_sConfig.asChannel[i].u8Unit;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PUBLIC void Hourly_Init(uint8_t configIsOk)
{
	init();
	if(configIsOk)
	{
		Archive_Init(&hourly);
	}
	else
	{
		Archive_Reset(&hourly);
	}
}

PUBLIC void Hourly_Write(tsArchiveRecordPack* precord)
{
	Archive_Write(&hourly, precord);
}

PUBLIC tsArchive* Hourly_GetParameters(void)
{
	return &hourly;
}

PUBLIC uint8_t Hourly_GetPage(void)
{
	return Archive_GetPage(&hourly);
}

PUBLIC uint8_t Hourly_GetPageIndex(void)
{
	return Archive_GetPageIndex(&hourly);
}

PUBLIC ErrorStatus Hourly_GetByPageOffset(tsArchiveRecordUnpack *pRecord, uint8_t page, uint8_t record)
{
	if(pRecord == NULL) return ERROR;
	tsArchiveRecordPack sFrom;
	sFrom.u32Ts = TIMESTAMP_UNDEFINED;
	Archive_Get(&sFrom, sizeof(tsArchiveRecordPack), &hourly, page, record);

	if(sFrom.u32Ts == 0xFFFFFFFF) return ERROR;
	_unpackRecord(pRecord, &sFrom);
	return SUCCESS;
}

PUBLIC ErrorStatus Hourly_GetByOffset(tsArchiveRecordUnpack* pRecord, uint16_t offset)
{
	if(pRecord == NULL) return ERROR;
	tsArchiveRecordPack sFrom;
	sFrom.u32Ts = TIMESTAMP_UNDEFINED;
	Archive_GetOffset(&sFrom, sizeof(tsArchiveRecordPack), &hourly, offset);

	if(sFrom.u32Ts == TIMESTAMP_UNDEFINED) return ERROR;
	_unpackRecord(pRecord, &sFrom);
	return SUCCESS;
}

PUBLIC uint32_t Hourly_GetPageAbs(uint8_t page)
{
	return Archive_GetPageAbs(&hourly, page);
}

////

PUBLIC tsArchiveRecordPack Hourly_MakeRecord(uint32_t time)
{
	tsArchiveRecordPack record;
	record.u32Ts = time;

	uint8_t i;
	for(i = 0; i < COUNTERS; i++)
	{
		record.au32Counter[i] = Counter_GetValue(i, &g_sConfig.asChannel[i]);
	}

	//record.u16PinState0 = GpioCtrl_GetPins() & 0x0000FFFF;
	//record.u16PinState1 = (GpioCtrl_GetPins() & 0xFFFF0000) >> 16;

	return record;
}

#endif
