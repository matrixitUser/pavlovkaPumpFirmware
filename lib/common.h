#ifndef __COMMON_H__
#define __COMMON_H__

#include "main.h"

#define RCC_APB1PeriphClockSet(do_enable, do_disable)		do{ RCC_APB1PeriphClockCmd(do_enable, ENABLE); RCC_APB1PeriphClockCmd(do_disable, DISABLE); } while(0)
#define RCC_APB2PeriphClockSet(do_enable, do_disable)		do{ RCC_APB2PeriphClockCmd(do_enable, ENABLE); RCC_APB2PeriphClockCmd(do_disable, DISABLE); } while(0)

PUBLIC uint32_t Override_RTC_GetCounter(void);
PUBLIC GPIO_TypeDef* Common_GetPortBySrc(u8 portsrc);
PUBLIC void memrcpy(void *dst, void *src, u16 len);
PUBLIC void memrcpy_u16(u8 *target, u16 src);
PUBLIC void memrcpy_u32(u8 *target, u32 src);
PUBLIC void memcpy_u16(uint8_t *target, uint16_t src);
PUBLIC void memcpy_u32(uint8_t *target, uint32_t src);
PUBLIC u32 memrget_u32(u8 *src);
PUBLIC u32 memget_u32(u8 *src);
PUBLIC u16 memrget_u16(u8 *src);
PUBLIC u16 memget_u16(u8 *src);
PUBLIC void delay_us(unsigned long n);

#endif
