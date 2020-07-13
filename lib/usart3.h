#ifndef __USART3_H__
#define __USART3_H__

#include "main.h"
#include "usart_v3.h"

#ifndef USART1_IRQ
#error Usart1 IRQ is UNDEFINED!
#endif

PUBLIC uint8_t 	Usart3_IsInWork(void);
PUBLIC void 			Usart3_Init(tsUartConfig *cfg);
PUBLIC uint8_t 	Usart3_Subscribe(ACTION_DATA(receive));
PUBLIC void 		Usart3_send(uint8_t *buf, uint16_t count);
PUBLIC void 		Usart3_sendStr(char *str);
PUBLIC void 			Usart3_sendDec(uint16_t num, uint8_t dec);
PUBLIC void 			Usart3_sendHex(uint32_t num, uint8_t dec);

#endif
