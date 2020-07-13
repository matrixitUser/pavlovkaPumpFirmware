#ifndef __UART4_H__
#define __UART4_H__

#include "main.h"
#include "usart_v3.h"

#ifndef UART4_IRQ
#error UART4 IRQ is UNDEFINED!
#endif

PUBLIC uint8_t 	UART4_IsInWork(void);
PUBLIC void 			UART4_Init(tsUartConfig *cfg);
PUBLIC uint8_t 	UART4_Subscribe(ACTION_DATA(receive));
PUBLIC void 		UART4_send(uint8_t *buf, uint16_t count);
PUBLIC void 		UART4_sendStr(char *str);
PUBLIC void 			UART4_sendDec(uint16_t num, uint8_t dec);
PUBLIC void 			UART4_sendHex(uint32_t num, uint8_t dec);

#endif
