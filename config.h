#include "main.h"
#define UNDEFINED4BYTE  0xFFFFFFFF
#if 1  //(defined(__FLASH_H__) && defined(__COUNTER_H__) && defined(__ARCHIVE_H__) && defined(__ABNORMAL_H__) && defined(__SWALARM_H__))
	#ifndef __CONFIG_H__
		#define __CONFIG_H__

		//#include "flash.h"
		////#include "usart_v3.h"
		//#include "counter.h"
		///#include "abnormal.h"
		//#include "archive.h"
		//#include "swalarm_v2.h"

		typedef enum
		{
			RESET_NONE,
			RESET_SOFT,
			RESET_HARD
		}
		ResetType_e;

		#define CONFIG CONFIG

		#define FLASH_VERSION			((((TYPE) & 0x1f) << 3)|((VERSION) & 0x07))
		#define FLASH_HELLO				(((FLASH_VERSION)<<8) | (sizeof(tsConfig) & 0xFF))


		PUBLIC u8 				Config_Init(void);
		PUBLIC void 			Config_Reset(void);
		PUBLIC u8				CheckConfig(tsConfig *psConfig);
		PUBLIC u8				CheckConfigNew(tsConfig *psConfig);
		PUBLIC void				ReadConfig(void);
		PUBLIC u8 				Config_Set(tsConfig *psConfig, ResetType_e eReset);

	#endif
#endif

