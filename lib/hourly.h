#include "main.h"
#if (defined(__ARCHIVE_H__) && defined(__COUNTER_H__))

	#ifndef __HOURLY_H__
		#define __HOURLY_H__

		#include "archive.h"

		#if (FLAGS & FLAGS_HOURLY_DEBUG) > 0
		#define HOURLY_INTERVAL				INTERVAL(0,0,64)
		#else
		#define HOURLY_INTERVAL				INTERVAL(1,0,0)
		#endif

		//#define HOURLY_INTERVAL				INTERVAL(1,0,0)
		#define HOURLY_INTERVAL_MSEC		INTERVAL_MSEC(0,0,HOURLY_INTERVAL,0)

		PUBLIC void 					Hourly_Init(uint8_t configIsOk);
		PUBLIC void 					Hourly_Write(tsArchiveRecordPack* precord);
		PUBLIC tsArchive* 				Hourly_GetParameters(void);
		PUBLIC u8 						Hourly_GetPage(void);
		PUBLIC u8 						Hourly_GetPageIndex(void);
		PUBLIC u32 						Hourly_GetPageAbs(u8 page);
		PUBLIC ErrorStatus 				Hourly_GetByPageOffset(tsArchiveRecordUnpack *pRecord, u8 page, u8 record);
		PUBLIC ErrorStatus 				Hourly_GetByOffset(tsArchiveRecordUnpack* pRecord, u16 offset);
		PUBLIC tsArchiveRecordPack 		Hourly_MakeRecord(uint32_t time);

	#endif
#endif
