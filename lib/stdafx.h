#ifndef __STDAFX_H__
#define __STDAFX_H__


#define TRUE				(1)
#define FALSE				(0)

#define TIMESTAMP_UNDEFINED (0xFFFFFFFFUL)
#define WORD_UNDEFINED 		(0xFFFFFFFFUL)

#define GET_TS_HOUR(x)		((x%(24*3600))/3600)
#define GET_TS_MIN(x)		((x%3600)/60)
#define GET_TS_SEC(x)		((x%3600)%60)

#define SETBITS(x,y) 	(x)|=(y)
#define SETBIT(x,y) 	SETBITS(x,1<<(y))
#define CLRBITS(x,y)	(x)&=~(y)
#define CLRBIT(x,y) 	CLRBITS(x,1<<(y))

#define CHECK_BIT(x,bit)	((x) & (1<<bit))
#define CHECK_BITS(x,bits)	((x) & (bits))

#define MIN(x,y)		((x)<(y))?(x):(y)

#define HIGH(x)			((x)>>8)
#define LOW(x)			((x)&0xFF)
#define TO_UINT16(low, hi)	( ( (low) & 0xFF) | ( ((uint16_t)(hi)) << 8) )

#define NO_OPTIMIZE		volatile
#define PRIVATE 		static
#define PUBLIC

#define ACTION(x) 		void (*x)(void)
#define ACTION1(x) 		void (*x)(void *)
#define ACTION_u8_u8(x) 	void (*x)(u8, u8)

#define ACTION_DATA(x)			void (*x)(u8*, u16)
#define ACTION_DATA_CB(x)		void (*x)(u8*, u16, ACTION_DATA(cb))

#define ACTION_CALLBACK(x) 	void (*x)(u8, u32)

#define INTERVAL(HH,MM,SS)	((HH)*60*60 + (MM)*60 + (SS))
#define INTERVAL_MSEC(HH,MM,SS,MS)	((HH)*60*60*1000 + (MM)*60*1000 + (SS)*1000 + (MS))


#define SEC(ts)		((ts) % 60)
#define MINUTE(ts)	(((ts) % 3600) / 60)
#define HOUR(ts)	(((ts) % 86400) / 3600)
#define DATE(ts)	((ts) / 86400)
#define DAY(ts)		(DATE(ts) % 7)

#endif
