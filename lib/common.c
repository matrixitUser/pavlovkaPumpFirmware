#include "common.h"

uint32_t Override_RTC_GetCounter(void)
{
  uint32_t counter;
  uint16_t tmp;

  do
  {
    tmp = RTC->CNTH;
    counter = (((uint32_t)tmp << 16 ) | RTC->CNTL);
  } while(tmp != RTC->CNTH);

  return counter;
}

PUBLIC GPIO_TypeDef* Common_GetPortBySrc(u8 portsrc)
{
	switch(portsrc)
	{
	case GPIO_PortSourceGPIOA:
		return GPIOA;

	case GPIO_PortSourceGPIOB:
		return GPIOB;

	case GPIO_PortSourceGPIOC:
		return GPIOC;

	case GPIO_PortSourceGPIOD:
		return GPIOD;

	case GPIO_PortSourceGPIOE:
		return GPIOE;

	case GPIO_PortSourceGPIOF:
		return GPIOF;

	case GPIO_PortSourceGPIOG:
		return GPIOG;
	}

	return NULL;
}

PUBLIC void memrcpy(void *dst, void *src, u16 len)
{
	u16 i;
	for(i=0;i<len;i++)
	{
		((u8 *)dst)[i]=((u8 *)src)[(len-1)-i];
	}
}

PUBLIC void memrcpy_u16(u8 *target, u16 src)
{
	target[1] = (u8)(src>>0);
	target[0] = (u8)(src>>8);
}



//что делает функция
PUBLIC void memrcpy_u32(u8 *target, u32 src)
{
	target[3] = (u8)(src>> 0);
	target[2] = (u8)(src>> 8);
	target[1] = (u8)(src>>16);
	target[0] = (u8)(src>>24);
}

PUBLIC void memcpy_u16(uint8_t *target, uint16_t src)
{
	target[0] = (uint8_t)(src >> 0);
	target[1] = (uint8_t)(src >> 8);
}

PUBLIC void memcpy_u32(uint8_t *target, uint32_t src)
{
	target[0] = (uint8_t)(src >> 0);
	target[1] = (uint8_t)(src >> 8);
	target[2] = (uint8_t)(src >> 16);
	target[3] = (uint8_t)(src >> 24);
}

PUBLIC u32 memrget_u32(u8 *src)
{
	return ((u32)src[0] << 24) + ((u32)src[1] << 16) + ((u32)src[2] << 8) + (src[3]);
}

PUBLIC u32 memget_u32(u8 *src)
{
	return ((u32)src[3] << 24) + ((u32)src[2] << 16) + ((u32)src[1] << 8) + (src[0]);
}

PUBLIC u16 memrget_u16(u8 *src)
{
	return ((u16)src[0] << 8) + (src[1]);
}

PUBLIC u16 memget_u16(u8 *src)
{
	return ((u16)src[1] << 8) + (src[0]);
}

PUBLIC void delay_us(unsigned long n)
{
	volatile unsigned long i;

	while(n--)
	{
		i=2;
		while(i--);
	}
}

