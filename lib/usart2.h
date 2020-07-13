#ifndef __USART2_H__
#define __USART2_H__

#include "main.h"
#include "usart_v3.h"

#ifndef USART2_IRQ
#error Usart2 IRQ is UNDEFINED!
#endif

PUBLIC uint8_t 	Usart2_IsInWork(void);
PUBLIC void 			Usart2_Init(tsUartConfig *cfg);
PUBLIC uint8_t 	Usart2_Subscribe(ACTION_DATA(receive));
PUBLIC void 		Usart2_send(uint8_t *buf, uint16_t count);
PUBLIC void 		Usart2_sendStr(char *str);
PUBLIC void 			Usart2_sendDec(uint16_t num, uint8_t dec);
PUBLIC void 			Usart2_sendHex(uint32_t num, uint8_t dec);

#endif
