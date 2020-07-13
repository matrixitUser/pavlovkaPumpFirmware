#ifndef __US4ART1_H__
#define __USART1_H__

#include "main.h"
#include "usart_v3.h"

#ifndef USART1_IRQ
#error Usart1 IRQ is UNDEFINED!
#endif

PUBLIC uint8_t 	Usart1_IsInWork(void);
PUBLIC void 			Usart1_Init(tsUartConfig *cfg);
PUBLIC uint8_t 	Usart1_Subscribe(ACTION_DATA(receive));
PUBLIC void 		Usart1_send(uint8_t *buf, uint16_t count);
PUBLIC void 		Usart1_sendStr(char *str);
PUBLIC void 			Usart1_sendDec(uint16_t num, uint8_t dec);
PUBLIC void 			Usart1_sendHex(uint32_t num, uint8_t dec);

#endif
