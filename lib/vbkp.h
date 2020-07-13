#ifndef __VBKP_H__
	#include "main.h"

	#if (defined(__BKP16_H__) && defined(__FLASH_H__))

		#define __VBKP_H__

		#define VBKP_TOTAL	16
		#if (VBKP_TOTAL == 12)
		#define VBKP_BKP12
		#define VBKP_FBKP_BITMASK	0xFFFFF000
		#elif (VBKP_TOTAL == 16)
		#define VBKP_BKP8
		#define VBKP_FBKP_BITMASK	0xFFFFFF00
		#else
		#error "Неправильное количество VBKP"
		#endif

		PUBLIC void VBkp_mInit(void);
		PUBLIC u32 	VBkp_Read(u8 n);
		PUBLIC void VBkp_Increment(u8 n);
		PUBLIC void VBkp_Set(u8 n, u32 value);
		PUBLIC void VBkp_mUpdate(void);

	#endif
#endif
