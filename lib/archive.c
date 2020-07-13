#include "main.h"

#ifdef __ARCHIVE_H__
#if (FLAGS & FLAGS_ENABLE_DEBUG) > 0
#include "usart_v2.h"
#endif

////часовой архив
//PRIVATE u8 page0 = 0xFF; // (текущая страница)
//PRIVATE u8 relx0 = 0xFF; // текущая запись (на текущей странице)
//PRIVATE u8 page1;		// (страница следующей записи)
//PRIVATE u8 relx1;		// следующая запись
//PRIVATE u32 page1_abs;	// абсолютный номер страницы


PUBLIC void Archive_Reset(tsArchive *parch)
{
	parch->_page_records = (( HW_FLASH_SIZE - 4 ) / parch->record_size);

	u8 i;
	u32* p = (u32*)(parch->start_page_addr);

	for(i = 0; i < parch->pages; i++) //обнуление страниц
	{
		Flash_Write4((u32)&p[((u32)i)<<8], NULL, 0, 1);
	}

	//обнуление параметров
	parch->_page0 = 0xFF;
	parch->_relx0 = 0xFF;

	parch->_page1_abs = 0;
	parch->_page1 = 0;
	parch->_relx1 = 0;

	u32 write = 0; //абсолютный номер = 0
	Flash_Write4(parch->start_page_addr, &write, sizeof(write), 1);
}

PUBLIC void Archive_Init(tsArchive *parch)
{
	parch->_page_records = (( HW_FLASH_SIZE - 4 ) / parch->record_size);

	u8 i;
	u32* p = (u32*)(parch->start_page_addr);
	u32 p0_abs = 0;
	u32 p1_abs = 0;
	for(i = 0; i < parch->pages; i++)//поиск последней используемой страницы
	{
		u8 i1 = i+1;
		if(i1 == parch->pages)
		{
			i1=0;
		}

		p0_abs = p[((u32)i)<<8];
		p1_abs = p[((u32)i1)<<8];

		if(p1_abs==0xFFFFFFFF || p1_abs<p0_abs || p1_abs-p0_abs!=1)
		{
			break;
		}
	}

	parch->_page1_abs = p0_abs;
	parch->_page0 = i;

	//поиск последней записи
	//предполагаем, что есть хотя бы одна запись на странице, либо страница полная
	u32* pcur = &p[(((u32)parch->_page0)<<8)+1];
	parch->_relx0 = 0;////? todo dododod
	for(i = parch->_page_records; i>0; i--)
	{
		if(pcur[(i-1)*(parch->record_size>>2)+0] != 0xFFFFFFFF)		//проверка: первые 4 байта записи
		{
			parch->_relx0 = (i-1);
			break;
		}
	}

	//вычисление следующего адреса
	parch->_relx1 = parch->_relx0+1;

	parch->_page1 = parch->_page0;
	if(parch->_relx1 == parch->_page_records)
	{
		parch->_page1_abs++;
		parch->_page1_abs &= 0x7FFFFFFF;
		parch->_page1++;
		if(parch->_page1 == parch->pages)
		{
			parch->_page1 = 0;
		}
		parch->_relx1 = 0;
	}
}

PUBLIC void Archive_Write(tsArchive *parch, void *record)
{
	u32 page_addr = parch->start_page_addr + (u32)parch->_page1*HW_FLASH_SIZE;

	//если предыдущая страница полна - стереть следующую и записать в нее значения
	if(parch->_relx1 == 0)
	{
		Flash_Write4(page_addr, &parch->_page1_abs, sizeof(u32), 1);
	}

	u32 addr_inx = page_addr + 4 + (u32)parch->_relx1 * parch->record_size;
	Flash_Write4(addr_inx, record, parch->record_size, 0);

#if (FLAGS & FLAGS_ENABLE_DEBUG) > 0
	Usart_sendStr(parch->name);
	Usart_sendStr(" arch write ");
	Usart_sendDec(parch->_page1, 2);
	Usart_sendStr(",");
	Usart_sendDec(parch->_relx1, 2);
	Usart_sendStr(" TS=0x");
	Usart_sendHex(((u32 *)record)[0], 8);
	Usart_sendStr("\r\n");
#endif

	//инкремент
	parch->_relx0 = parch->_relx1;
	parch->_relx1++;

	parch->_page0 = parch->_page1;
	if(parch->_relx1 == parch->_page_records)
	{
		parch->_page1_abs++;
		parch->_page1_abs &= 0x7FFFFFFF;
		parch->_page1++;
		if(parch->_page1 == parch->pages)
		{
			parch->_page1=0;
		}
		parch->_relx1=0;
	}
}


PUBLIC u8 Archive_GetPage(tsArchive *parch)
{
	return parch->_page0;
}

PUBLIC u8 Archive_GetPageIndex(tsArchive *parch)
{
	return parch->_relx0;
}

PUBLIC u32 Archive_GetPageAbs(tsArchive *parch, u8 page)
{
	if(page < parch->pages)
	{
		return *(u32*)(parch->start_page_addr + (u32)page * HW_FLASH_SIZE);
	}
	return 0xFFFFFFFF;
}


PUBLIC ErrorStatus Archive_Get(void *to, uint16_t len, tsArchive *parch, u8 page, u8 record)
{
	if((to != NULL) && (len != 0) && (page < parch->pages) && (record < parch->_page_records))
	{
		memcpy(to, (uint8_t *)(parch->start_page_addr + ((u32)page * HW_FLASH_SIZE) + 4 + (u16)record * parch->record_size), len);
		return SUCCESS;
	}
	return ERROR;
}


PUBLIC ErrorStatus Archive_GetOffset(void *to, uint16_t len, tsArchive *parch, u16 offset)//поиск записи с отступом
{
	if((to == NULL) || (len == 0)) return ERROR;
	if(offset < (parch->pages * parch->_page_records))		//если отступ не превышает максимальное количество записей в архиве
	{
		u8 page = parch->_page0;
		u8 record = parch->_relx0;
		u16 i;
		for(i=0; i<(offset+1); i++)							//поиск записи по циклическому архиву назад
		{
			if(i == offset)									//запись найдена
			{
				Archive_Get(to, len, parch, page, record);
				return SUCCESS;
			}

			if(record==0)
			{
				record = parch->_page_records - 1;
				if(page==0)
				{
					page = parch->pages - 1;
				}
				else
				{
					page--;
				}
			}
			else
			{
				record--;
			}
		}
	}
	return ERROR;
}

#endif
