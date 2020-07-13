#include "main.h"

#ifdef __VBKP_H__
volatile uint32_t _vbkp_cur[VBKP_TOTAL];//текущее VBKP в RAM
PRIVATE uint8_t _vbkp_chg;//флаги изменени€ группы BKP16 по 3
PRIVATE uint16_t _vbkp_wflh;//флаги записи во флеш-пам€ть FBKP+BKP8
//

#define VBKP_START_PAGE_ADDR	(VBKP_PAGE)
#define VBKP_RECORD_SIZE		(4)

//в заголовке наход€тс€ начальные значени€
#define VBKP_HEADER_SIZE		(4 + (VBKP_TOTAL) * VBKP_RECORD_SIZE)
//всего записей на страницу
#define VBKP_PAGE_RECORDS		(( HW_FLASH_SIZE - VBKP_HEADER_SIZE ) / VBKP_RECORD_SIZE)

/////////////////////////////////// FLASH
PRIVATE uint8_t __page0 = 0xFF;
PRIVATE uint8_t __relx0 = 0xFF;
PRIVATE uint8_t __page1 = 0;
PRIVATE uint8_t __relx1 = 0;
PRIVATE uint32_t __page1_abs;

//1. перва€ инициализаци€
//	 инициализируем FBKP нул€ми
//	 заполн€ем нулевую страницу и заголовок

//2. переинициализаци€ (после перезагрузки)
//	 загружаем значени€ FBKP

//»нициализаци€ FBKP
PRIVATE uint8_t _FBKP_Init(void)
{
	uint8_t i;
	u32* p = (u32*)(VBKP_START_PAGE_ADDR);

	uint32_t p0_abs = p[0];
	uint32_t p1_abs;

	if(p0_abs == 0xFFFFFFFF) 			//1. перва€ инициализаци€. переменные = 0
	{
#if (FLAGS & FLAGS_ENABLE_DEBUG) > 0
		Usart_sendStr("1st init\r\n");
#endif

		__page1 = 0;
		__relx1 = 0;
		__page1_abs = 0;

		for(i=0; i<VBKP_TOTAL; i++)		//обнуление VBKP
		{
			_vbkp_cur[i] = 0;
		}
		return 0;
	}

	//2. инициализаци€. загрузка переменных из флеш
	for(i=0; i<VBKP_PAGES; i++)		//поиск последней используемой страницы
	{
		uint8_t i1 = i+1;
		if(i1 == VBKP_PAGES)
		{
			i1=0;
		}

		p1_abs = p[((u32)i1)<<8];

		if(p1_abs==0xFFFFFFFF || p1_abs<p0_abs || p1_abs-p0_abs!=1)
		{
			break;
		}

		p0_abs = p1_abs;
	}

#if (FLAGS & FLAGS_ENABLE_DEBUG) > 0
	Usart_sendStr("Init PAGE=");
	Usart_sendDec(i, 1);
	Usart_sendStr("/");
	Usart_sendDec((u16)p0_abs, 5);
	Usart_sendStr("\r\n");
#endif

	__page1_abs = p0_abs;
	__page0 = i;

	//получение информации
	// тут надо прочесть страницу и узнать последние значени€

	//предполагаем, что есть хот€ бы одна запись на странице, либо страница полна€
	u32* phead = &p[(((u32)__page0)<<8) + 1];
	u32* pcur = &p[(((u32)__page0)<<8) + (VBKP_HEADER_SIZE>>2)];

#if (FLAGS & FLAGS_ENABLE_DEBUG) > 0
	Usart_sendStr("FBKP");
	Usart_sendStr("\r\n");
#endif
	for(i=0; i<VBKP_TOTAL; i++)
	{
		uint32_t prec = phead[i*(VBKP_RECORD_SIZE>>2)];
		if((prec != 0xFFFFFFFF) && ((prec & 0xFF) < VBKP_TOTAL))		//проверка: первые 4 байта записи не должны быть пустыми
		{
			_vbkp_cur[i] = prec & VBKP_FBKP_BITMASK;

#if (FLAGS & FLAGS_ENABLE_DEBUG) > 0
			Usart_sendDec(i, 2);
			Usart_sendStr(")=");
			Usart_sendHex((u32)(prec>>12), 5);
			Usart_sendStr("h\r\n");
#endif
		}
	}

	__relx0 = 0;
	for(i=0; i<VBKP_PAGE_RECORDS; i++)
	{
		uint32_t prec = pcur[i*(VBKP_RECORD_SIZE>>2)];
		if(prec == 0xFFFFFFFF) break;				//всЄ прочитано

		if((prec & 0xFF) < VBKP_TOTAL)				//проверка: первые 4 байта записи не должны быть пустыми
		{
			_vbkp_cur[prec & 0xFF] = prec & VBKP_FBKP_BITMASK;
			__relx0 = i;
		}
	}

#if (FLAGS & FLAGS_ENABLE_DEBUG) > 0
	Usart_sendStr("U");
	Usart_sendStr("\r\n");
	for(i=0; i<VBKP_TOTAL; i++)
	{
		Usart_sendDec(i, 2);
		Usart_sendStr(")=");
		Usart_sendHex((u32)(_vbkp_cur[i]>>12), 5);
		Usart_sendStr("h\r\n");
	}
#endif

	//вычисление следующего адреса
	__relx1 = __relx0 + 1;

	__page1 = __page0;
	if(__relx1 == VBKP_PAGE_RECORDS)
	{
		__page1_abs++;
		__page1_abs &= 0x7FFFFFFF;
		__page1++;
		if(__page1 == VBKP_PAGES)
		{
			__page1 = 0;
		}
		__relx1 = 0;
	}
	return 1;
}

//запись флеш
PUBLIC void _FBKP_Write(uint8_t n, uint32_t value)
{
	uint32_t record = (value & VBKP_FBKP_BITMASK) | n;
	uint32_t page_addr = VBKP_START_PAGE_ADDR + (u32)__page1*HW_FLASH_SIZE;

	//если предыдуща€ страница полна - стереть следующую и записать в нее заголовок
	if(__relx1 == 0)
	{
		Flash_Write4(page_addr, &__page1_abs, sizeof(u32), 1);

		uint8_t i;
		for(i=0; i<VBKP_TOTAL; i++)
		{
			uint32_t addr_inx = page_addr + 4 + i * VBKP_RECORD_SIZE;
			uint32_t record = (_vbkp_cur[i] & VBKP_FBKP_BITMASK) | i;
			Flash_Write4(addr_inx, &record, VBKP_RECORD_SIZE, 0);
		}
	}

	uint32_t addr_inx = page_addr + VBKP_HEADER_SIZE + (u32)__relx1 * VBKP_RECORD_SIZE;
	Flash_Write4(addr_inx, &record, VBKP_RECORD_SIZE, 0);

#if (FLAGS & FLAGS_ENABLE_DEBUG) > 0
	Usart_sendStr("write ");
	Usart_sendDec(__page1, 1);
	Usart_sendStr(",");
	Usart_sendDec(__relx1, 3);
	Usart_sendStr("\r\n");
#endif

	//инкремент
	__relx0 = __relx1;
	__relx1++;

	__page0 = __page1;
	if(__relx1 == VBKP_PAGE_RECORDS)
	{
		__page1_abs++;
		__page1_abs &= 0x7FFFFFFF;
		__page1++;
		if(__page1 == VBKP_PAGES)
		{
			__page1=0;
		}
		__relx1=0;
	}
}

//////////////////////////////////////////

//ѕри загрузке нужно восстановить значение VBKP в RAM.
PUBLIC void VBkp_mInit(void)
{
	volatile uint8_t i;

	//—начала читаем из флеш
	_FBKP_Init();

	//прибавл€ем BKP-регистр
#if defined(VBKP_BKP12)
	for(i=0; i<(VBKP_TOTAL>>2); i++)
	{
		uint16_t bkp1 = Bkp16_Read(i*3+0);
		uint16_t bkp2 = Bkp16_Read(i*3+1);
		uint16_t bkp3 = Bkp16_Read(i*3+2);

		_vbkp_cur[(i<<2) + 0] |= (0)					| ((bkp1) >> 4); 			//FFF0 0000 0000 => 0x0FFF
		_vbkp_cur[(i<<2) + 1] |= ((bkp1 & 0x000F) << 8) | ((bkp2 & 0xFF00) >> 8); 	//000F FF00 0000 => 0x0FFF
		_vbkp_cur[(i<<2) + 2] |= ((bkp2 & 0x00FF) << 4)	| ((bkp3) >> 12);			//0000 00FF F000 => 0x0FFF
		_vbkp_cur[(i<<2) + 3] |= ((bkp3 & 0x0FFF) << 0) | (0);						//0000 0000 0FFF => 0x0FFF

#if (FLAGS & FLAGS_ENABLE_DEBUG) > 0
		Usart_sendStr("(");
		Usart_sendDec((i<<2) + 0, 2);
		Usart_sendStr(")=");
		Usart_sendHex(_vbkp_cur[(i<<2) + 0], 8);
		Usart_sendStr(", (");
		Usart_sendDec((i<<2) + 1, 2);
		Usart_sendStr(")=");
		Usart_sendHex(_vbkp_cur[(i<<2) + 1], 8);
		Usart_sendStr(", (");
		Usart_sendDec((i<<2) + 2, 2);
		Usart_sendStr(")=");
		Usart_sendHex(_vbkp_cur[(i<<2) + 2], 8);
		Usart_sendStr(", (");
		Usart_sendDec((i<<2) + 3, 2);
		Usart_sendStr(")=");
		Usart_sendHex(_vbkp_cur[(i<<2) + 3], 8);
		Usart_sendStr("\r\n");
	}
#endif
#elif defined(VBKP_BKP8)
	for(i = 0; i < (VBKP_TOTAL >> 1); i++)
	{
		uint16_t bkp = Bkp16_Read(i);
		_vbkp_cur[(i << 1) + 0] |= ((bkp & 0xFF00) >> 8);
		_vbkp_cur[(i << 1) + 1] |= (bkp & 0x00FF);
	}
#else
#error "–азр€дность BKP не установлена"
#endif
}

PUBLIC uint32_t VBkp_Read(uint8_t n)
{
	if(n >= VBKP_TOTAL) return 0;
	return _vbkp_cur[n];
}

PUBLIC void VBkp_Increment(uint8_t n)//0~15
{
	if(n >= VBKP_TOTAL) return;

	//»нкрементируем VBKP(RAM).
	++_vbkp_cur[n];

#if defined(VBKP_BKP12)
	//‘лаг записи во флеш
	if((_vbkp_cur[n] & 0x0FFF) == 0)
	{
		SETBIT(_vbkp_wflh, n);
#if (FLAGS & FLAGS_ENABLE_DEBUG) > 0
		Usart_sendStr("Inc");
		Usart_sendDec(n, 2);
		Usart_sendStr(")=");
		Usart_sendHex((u32)(_vbkp_cur[n]>>12), 5);
		Usart_sendStr("|");
		Usart_sendHex((u32)(_vbkp_cur[n] & 0x0FFF), 3);
		Usart_sendStr(" owf");
		Usart_sendStr("\r\n");
#endif
	}

	//‘лаг изменени€
	if(n <= 2) {
		SETBIT(_vbkp_chg, 0);
	} else if(n > 5) {
		SETBIT(_vbkp_chg, 2);
	} else {
		SETBIT(_vbkp_chg, 1);
	}
#elif defined(VBKP_BKP8)
	//‘лаг записи во флеш
	if((_vbkp_cur[n] & 0xFF) == 0)
	{
		SETBIT(_vbkp_wflh, n);
#if (FLAGS & FLAGS_ENABLE_DEBUG) > 0
		Usart_sendStr("Inc");
		Usart_sendDec(n, 2);
		Usart_sendStr(")=");
		Usart_sendHex((u32)(_vbkp_cur[n]>>12), 5);
		Usart_sendStr("|");
		Usart_sendHex((u32)(_vbkp_cur[n] & 0x0FFF), 3);
		Usart_sendStr(" owf");
		Usart_sendStr("\r\n");
#endif
	}

	SETBIT(_vbkp_chg, (n>>1));
#else
#error "–азр€дность BKP не установлена"
#endif
}

PUBLIC void VBkp_Set(uint8_t n, uint32_t value)//0..15
{
	if(n >= VBKP_TOTAL) return;

	//»нкрементируем VBKP(RAM).
	uint32_t old = _vbkp_cur[n];
	_vbkp_cur[n] = value;

#if defined(VBKP_BKP12)
#elif defined(VBKP_BKP8)
	//‘лаг записи во флеш
	if((old >> 8) != (value >> 8))
	{
		SETBIT(_vbkp_wflh, n);
	}

	//‘лаг изменени€
	SETBIT(_vbkp_chg, (n>>1));
#else
#error "–азр€дность BKP не установлена"
#endif
}

//!ќбновление в главном цикле!
PUBLIC void VBkp_mUpdate(void)
{
#if defined(VBKP_BKP12)
	uint8_t i, j;
	for(i=0; i<(VBKP_TOTAL>>2); i++) // 0~2
	{
		if(CHECK_BIT(_vbkp_chg, i))
		{
			CLRBIT(_vbkp_chg, i);

			uint16_t bkp16Value1 = 0;
			uint16_t bkp16Value2 = 0;
			uint16_t bkp16Value3 = 0;

			for(j=0; j<4; j++)//0~3
			{
				uint8_t n = (i<<2) + j; //0~11
				uint32_t vbkpn = _vbkp_cur[n];
				uint16_t bkp12 = vbkpn & 0x0FFF;

				//ѕолучаем значение BKP
				switch(j)
				{
				case 0:
					bkp16Value1 |= (bkp12 << 4);
					break;
				case 1:
					bkp16Value1 |= (bkp12 >> 8);
					bkp16Value2 |= (bkp12 << 8);
					break;
				case 2:
					bkp16Value2 |= (bkp12 >> 4);
					bkp16Value3 |= (bkp12 << 12);
					break;
				case 3:
					bkp16Value3 |= bkp12;
					break;
				}

				//≈сли переполнилось, то пишем во флеш-пам€ть.
				if(CHECK_BIT(_vbkp_wflh, n))
				{
					CLRBIT(_vbkp_wflh, n);
					//
					_FBKP_Write(n, vbkpn);
				}
			}

			Bkp16_Write(i*3 + 0, bkp16Value1);
			Bkp16_Write(i*3 + 1, bkp16Value2);
			Bkp16_Write(i*3 + 2, bkp16Value3);
		}
	}
#elif defined(VBKP_BKP8)
	uint8_t i, j;
	for(i = 0; i < (VBKP_TOTAL >> 1); i++) // 0..7
	{
		if(CHECK_BIT(_vbkp_chg, i))
		{
			CLRBIT(_vbkp_chg, i);

			uint16_t bkp16Value = 0;

			for(j = 0; j < 2; j++)//0..1
			{
				uint8_t n = (i << 1) + j; //0..15
				uint32_t vbkpn = _vbkp_cur[n];
				uint16_t bkp8 = vbkpn & 0xFF;

				bkp16Value <<= 8;
				bkp16Value |= bkp8;

				//≈сли переполнилось, то пишем во флеш-пам€ть.
				if(CHECK_BIT(_vbkp_wflh, n))
				{
					CLRBIT(_vbkp_wflh, n);
					_FBKP_Write(n, vbkpn);
				}
			}

			Bkp16_Write(i, bkp16Value);
		}
	}
#else
#error "–азр€дность BKP не установлена"
#endif
}

#endif

