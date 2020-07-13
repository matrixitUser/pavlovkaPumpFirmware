#ifndef __ABNORMAL_H__

	#include "main.h"

	#if (defined(__ARCHIVE_H__))

		#define __ABNORMAL_H__

		//события
		#define AEID_RESTART_TYPE		0x00000000UL
		#define AEID_RESTART_PINRST		0x0001
		#define AEID_RESTART_PORRST		0x0002
		#define AEID_RESTART_SFTRST		0x0004
		#define AEID_RESTART_IWDGRST	0x0008
		#define AEID_RESTART_WWDGRST	0x0010
		#define AEID_RESTART_LPWRRST	0x0020

		#define AEID_SETBKP_TYPE		0x00010000UL
		#define AEID_SETBKP_TIMEMASK	0x1FFF
		#define AEID_SETBKP_SIGN		0x2000
		#define AEID_SETBKP_REGISTERS	0x4000

		#define AEID_TIME_OFF_TYPE		0x00020000UL

		#define AEID_CHANGE_MODE_TYPE	0x00030000UL

		#define AEID_BKP_RECOVER_TYPE	0x000F0000UL

		//исключительные ситуации
		#define AEID_FAULT_TYPE			0x80000000UL
		#define AEID_FAULT_HARD			0x0000
		#define AEID_FAULT_MEMMANAGE	0x0001
		#define AEID_FAULT_BUS			0x0002
		#define AEID_FAULT_USAGE		0x0003

		#define AEID_TYPE_MASK			0xFFFF0000UL

		PUBLIC void 					Abnormal_Init(uint8_t configIsOk);
		PUBLIC void 					Abnormal_Write(tsAbnormalEvent* precord);
		PUBLIC tsAbnormalEvent 			Abnormal_MakeRecord(uint32_t time, uint32_t aeid);

		PUBLIC uint8_t 					Abnormal_GetPage(void);
		PUBLIC uint8_t 					Abnormal_GetPageIndex(void);
		PUBLIC ErrorStatus 				Abnormal_GetByPageOffset(tsAbnormalEvent *pRecord, uint8_t page, uint8_t record);
		PUBLIC ErrorStatus 				Abnormal_GetByOffset(tsAbnormalEvent *pRecord, uint16_t offset);

		PUBLIC tsAbnormalEvent 			Abnormal_MakeRecord(uint32_t time, uint32_t aeid);
		PUBLIC uint16_t 				Abnormal_GetResetFlags(void);

	#endif
#endif
