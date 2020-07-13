#include "usart_v3.h"
#include "sleep_reset.h"

#define IS_USART_BAUDRATE_STANDARD(baud)	(((baud) == 4800) || ((baud) == 9600) || ((baud) == 19200) || ((baud) == 38400) || ((baud) == 115200))

PUBLIC uint8_t Usart_Config_Check(tsUartConfig *psUartConfig)
{
	if(	//!IS_USART_BAUDRATE(psUartConfig->u32BaudRate) ||
		!IS_USART_BAUDRATE_STANDARD(psUartConfig->u32BaudRate) ||
		!IS_USART_WORD_LENGTH((uint16_t)psUartConfig->u8WordLen << 8) ||
		!IS_USART_STOPBITS((uint16_t)psUartConfig->u8StopBits << 8) ||
		!IS_USART_PARITY((uint16_t)psUartConfig->u8Parity << 8))
	{
		return 0;
	}

	return 1;
}

PUBLIC void Usart_Config_SetDefault(tsUartConfig* psUartConfig, uint32_t baudRate)
{
	if(psUartConfig == NULL) return;

	psUartConfig->u32BaudRate = baudRate;
	psUartConfig->u8WordLen = (USART_WordLength_8b >> 8);
	psUartConfig->u8StopBits = (USART_StopBits_1 >> 8);
	psUartConfig->u8Parity = (USART_Parity_No >> 8);
}


PUBLIC void Usart_Init(USART_TypeDef *USARTx, Usart_t *psUsart, tsUartConfig *psUartConfig, uint8_t NVIC_IRQChannel)
{
    USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = psUartConfig->u32BaudRate;
	USART_InitStructure.USART_WordLength = (uint16_t)psUartConfig->u8WordLen << 8;
	USART_InitStructure.USART_StopBits = (uint16_t)psUartConfig->u8StopBits << 8;
	USART_InitStructure.USART_Parity = (uint16_t)psUartConfig->u8Parity << 8;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USARTx, &USART_InitStructure);

#ifdef CHIP_0XX
	USART_StopModeWakeUpSourceConfig(USARTx, USART_WakeUpSource_RXNE);
#endif

    /* Enable USART */
    USART_Cmd(USARTx, ENABLE);

#ifdef CHIP_0XX
    /* Before entering the USART in STOP mode the REACK flag must be checked to ensure the USART RX is ready */
    while(USART_GetFlagStatus(USARTx, USART_FLAG_REACK) == RESET)
    {}

    /* Enable USART STOP mode by setting the UESM bit in the CR1 register.*/
    USART_STOPModeCmd(USARTx, ENABLE);

    /* Enable the wake up from stop Interrupt */
    USART_ITConfig(USARTx, USART_IT_WU, ENABLE);
#endif

    //buffer init
    psUsart->pbTrsPos = psUsart->pbTrsEnd = psUsart->pbTrsBuf;
    psUsart->pbRcvPos = psUsart->pbRcvBuf;

    /* Enable the USART Interrupt */
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = NVIC_IRQChannel;
#ifdef CHIP_0XX
    NVIC_InitStructure.NVIC_IRQChannelPriority = NVIC_PRIORITY_USART;
#else
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_PRIORITY_PREEMPTIVE(NVIC_PRIORITY_USART);
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = IRQ_PRIORITY_NONPREEMPTIVE(NVIC_PRIORITY_USART);
#endif
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    //

	USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);
	USART_ITConfig(USARTx, USART_IT_TC, ENABLE);

	SETBIT(psUsart->flags, USART_FLAG_INWORK);
}

PUBLIC void Usart_send(USART_TypeDef *USARTx, Usart_t *psUsart, uint8_t *buf, uint16_t length)
{
	if(!CHECK_BIT(psUsart->flags, USART_FLAG_INWORK)) return;
	if( (length == 0) || (length > USART_TRS_MAX) ) return;
	if( (psUsart->pbTrsEnd + length) >= (psUsart->pbTrsBuf + USART_TRS_MAX) ) return;

	memcpy(psUsart->pbTrsEnd, buf, length);
	psUsart->pbTrsEnd += length;

	USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);
}

//*PUBLIC inline void Usart_receiveByte(USART_TypeDef *USARTx, Usart_t *psUsart)
PUBLIC void Usart_receiveByte(USART_TypeDef *USARTx, Usart_t *psUsart)
{
	USART_ITConfig(USARTx, USART_IT_IDLE, ENABLE);

    uint8_t cc = (uint8_t)USART_ReceiveData(USARTx);
    *psUsart->pbRcvPos = cc;

    if(psUsart->pbRcvPos < (psUsart->pbRcvBuf + USART_RCV_MAX))
    {
    	++psUsart->pbRcvPos;
    }
}

//PUBLIC inline void Usart_receiveCompleted(USART_TypeDef *USARTx, Usart_t *psUsart)
PUBLIC void Usart_receiveCompleted(USART_TypeDef *USARTx, Usart_t *psUsart)
{
	USART_ITConfig(USARTx, USART_IT_IDLE, DISABLE);

	if(psUsart->dataReceive != NULL)
	{
		psUsart->dataReceive(psUsart->pbRcvBuf, (psUsart->pbRcvPos - psUsart->pbRcvBuf));
	}

	psUsart->pbRcvPos = psUsart->pbRcvBuf;
}

//*PUBLIC inline void Usart_transmitByte(USART_TypeDef *USARTx, Usart_t *psUsart)
PUBLIC void Usart_transmitByte(USART_TypeDef *USARTx, Usart_t *psUsart)
{
    if(psUsart->pbTrsPos < psUsart->pbTrsEnd)
    {
    	USART_SendData(USARTx, psUsart->pbTrsPos[0]);
    	psUsart->pbTrsPos++;
        if(psUsart->pbTrsPos == psUsart->pbTrsEnd)
        {
            USART_ITConfig(USARTx, USART_IT_TXE, DISABLE);
            psUsart->pbTrsPos = psUsart->pbTrsEnd = psUsart->pbTrsBuf;
        }
    }
}

//PUBLIC inline void Usart_transmitComplete(USART_TypeDef *USARTx)
PUBLIC void Usart_transmitComplete(USART_TypeDef *USARTx)
{
#ifndef CHIP_0XX
    USART_ClearITPendingBit(USARTx, USART_IT_RXNE);
    USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);
#endif
}

/*
inline void Usart_sendStr(Usart_t* u, char *str)
{
	Usart_send(u, (uint8_t *)str, strlen(str));
}

PUBLIC void Usart_sendDec(Usart_t* u, uint16_t num, uint8_t dec)
{
	uint8_t buf[5];
	uint8_t i;
	uint16_t div;
	for(i=0,div=10000; div>0; div/=10, i++){
		buf[i] = num / div + '0';
		num %= div;
	}
	Usart_send(u, buf + 5 - dec, dec);
}

PUBLIC void Usart_sendHex(Usart_t* u, uint32_t num, uint8_t dec)
{
	uint8_t buf[8];
	uint8_t i;
	for(i=8; i>0; i--)
	{
		uint8_t hex = num & 0x0f;
		buf[i-1] = (hex >= 10)? (hex - 10) + 'A' : hex + '0';
		num >>= 4;
	}
	Usart_send(u, buf + 8 - dec, dec);
}
*/
