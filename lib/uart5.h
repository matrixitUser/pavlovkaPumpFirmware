#ifndef __UART5_H__
#define __UART5_H__

#include "main.h"
#include "usart_v3.h"

#ifndef UART5_IRQ
#error UART5 IRQ is UNDEFINED!
#endif

PUBLIC uint8_t 	UART5_IsInWork(void);
PUBLIC void 			UART5_Init(tsUartConfig *cfg);
PUBLIC uint8_t 	UART5_Subscribe(ACTION_DATA(receive));
PUBLIC void 		UART5_send(uint8_t *buf, uint16_t count);
PUBLIC void 		UART5_sendStr(char *str);
PUBLIC void 			UART5_sendDec(uint16_t num, uint8_t dec);
PUBLIC void 			UART5_sendHex(uint32_t num, uint8_t dec);

#endif
