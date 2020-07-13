
#ifndef __ARCHIVE_H__
#define __ARCHIVE_H__

#include "main.h"

typedef struct
{
	u8 _page0;	// (текущая страница)
	u8 _relx0;	// текущая запись (на текущей странице)
	u8 _page1;	// (страница следующей записи)
	u8 _relx1;	// следующая запись

	u32 _page1_abs;	// абсолютный номер страницы

	u32 start_page_addr;

	u16 _page_records;
	u8 record_size;
	u8 pages;

#if (FLAGS & FLAGS_ENABLE_DEBUG) > 0
	char name[10];
#endif
}
tsArchive;

PUBLIC void Archive_Reset(tsArchive *parch);
PUBLIC void Archive_Init(tsArchive *parch);
PUBLIC void Archive_Write(tsArchive *parch, void *record);

PUBLIC u8 Archive_GetPage(tsArchive *parch);
PUBLIC u8 Archive_GetPageIndex(tsArchive *parch);
PUBLIC u32 Archive_GetPageAbs(tsArchive *parch, u8 page);
PUBLIC ErrorStatus Archive_Get(void *to, uint16_t len, tsArchive *parch, u8 page, u8 record);
PUBLIC ErrorStatus Archive_GetOffset(void *to, uint16_t len, tsArchive *parch, u16 offset);

#endif
