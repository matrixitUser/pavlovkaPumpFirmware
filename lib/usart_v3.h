#ifndef __USART_H__
#define __USART_H__

#include "main.h"

#define USART_RCV_MAX			512
#define USART_TRS_MAX			512
#define USART_FLAG_INWORK		0

#define USART_EVENT_RECEIVE_COMPLETED 0

/*#define USART_BAUDRATE_MIN	120
#define USART_BAUDRATE_MAX	230400

#define IS_USART_BAUDRATE (BAUDRATE) (((BAUDRATE) >= USART_BAUDRATE_MIN) && \
                                      ((BAUDRATE) <= USART_BAUDRATE_MAX))*/


typedef struct Usart		//структура уарт(фрейм)
{
	ACTION_DATA(dataReceive);

	uint8_t *pbRcvPos;		//текущая позиция(курсор)
	uint8_t *pbTrsPos;		//текущая позиция передачи
	uint8_t *pbTrsEnd;		//позиция окончания передачи

	uint8_t flags;

	uint8_t  pbRcvBuf[USART_RCV_MAX];//буфер приема
	uint8_t  pbTrsBuf[USART_TRS_MAX];//буфер передачи
} Usart_t;

PUBLIC uint8_t Usart_Config_Check(tsUartConfig *psUartConfig);
PUBLIC void Usart_Config_SetDefault(tsUartConfig* psUartConfig, uint32_t baudRate);

PUBLIC void Usart_Init(USART_TypeDef *USARTx, Usart_t *psUsart, tsUartConfig *psUartConfig, uint8_t NVIC_IRQChannel);
PUBLIC void Usart_send(USART_TypeDef *USARTx, Usart_t *psUsart, uint8_t *buf, uint16_t length);

PUBLIC  void Usart_receiveByte(USART_TypeDef *USARTx, Usart_t *psUsart);
PUBLIC  void Usart_receiveCompleted(USART_TypeDef *USARTx, Usart_t *psUsart);
PUBLIC  void Usart_transmitByte(USART_TypeDef *USARTx, Usart_t *psUsart);
PUBLIC  void Usart_transmitComplete(USART_TypeDef *USARTx);

#endif
