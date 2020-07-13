#include "main.h"
#ifdef __UART4_H__
PRIVATE tsUartConfig sUartConfig;
PRIVATE Usart_t Usart;

PUBLIC uint8_t Uart4_IsInWork(void)
{
	return CHECK_BIT(Usart.flags, USART_FLAG_INWORK);
}

PUBLIC void UART4_Init(tsUartConfig *cfg)
{
	tsUartConfig *psUartConfig;
	if(cfg == NULL)
	{
		Usart_Config_SetDefault(&sUartConfig, UART4_SPEED);
		psUartConfig = &sUartConfig;
	}
	else
	{
		psUartConfig = cfg;
	}

    /* Enable GPIO clock */
	UART4_GPIO_CLOCK_SET(ENABLE);

    /* Enable USART clock */
	UART4_CLOCK_SET(ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

#ifdef CHIP_0XX
    /* Connect PXx to USARTx_Tx */
    GPIO_PinAFConfig(UART4_TX_PORT, UART4_TX_PSRC, UART4_TX_AFMODE);

    /* Connect PXx to USARTx_Rx */
    GPIO_PinAFConfig(UART4_RX_PORT, UART4_RX_PSRC, UART4_RX_AFMODE);

    /* Configure USART Tx, Rx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_2;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_InitStructure.GPIO_Pin = UART4_TX_PIN;
    GPIO_InitStructure.GPIO_Mode = UART4_TX_MODE_ON;
    GPIO_Init(UART4_TX_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = UART4_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = UART4_RX_MODE_ON;
    GPIO_Init(UART4_RX_PORT, &GPIO_InitStructure);

#ifdef UART4_RS485
    /* Connect PXx to USARTx_DE */
    GPIO_PinAFConfig(UART4_DE_PORT, UART4_DE_PSRC, UART4_DE_AFMODE);

    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = UART4_DE_PIN;
    GPIO_InitStructure.GPIO_Mode = UART4_DE_MODE_ON;
    GPIO_Init(UART4_DE_PORT, &GPIO_InitStructure);

    USART_DECmd(UART4, ENABLE);
    USART_DEPolarityConfig(UART4, USART_DEPolarity_High);
    //USART_SetDEAssertionTime(UART4, 4);
    //USART_SetDEDeassertionTime(UART4, 4);
#endif
#else
    GPIO_InitStructure.GPIO_Pin   = UART4_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode  = UART4_TX_MODE_ON;
    GPIO_Init(UART4_TX_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = UART4_RX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode  = UART4_RX_MODE_ON;
    GPIO_Init(UART4_RX_PORT, &GPIO_InitStructure);
#ifdef UART4_RS485

	//jtag -> pa15 поэтому след.  оператор
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    GPIO_InitStructure.GPIO_Pin   = UART4_DIR_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode  = UART4_DIR_MODE_ON;
    GPIO_Init(UART4_DIR_PORT, &GPIO_InitStructure);

	GPIO_ResetBits(UART4_DIR_PORT, UART4_DIR_PIN);
#endif

	Sleep_Config(SLEEP_FLAG_UART4_RX, SET);
#endif

    Usart_Init(UART4, &Usart, psUartConfig, UART4_IRQn);
}

PUBLIC uint8_t UART4_Subscribe(ACTION_DATA(receive))
{
	if(Usart.dataReceive == NULL)
	{
		Usart.dataReceive = receive;
		return 1;
	}
	return 0;
}

//PUBLIC inline void UART4_send(uint8_t *buf, uint16_t count)
PUBLIC void UART4_send(uint8_t *buf, uint16_t count)
{
#ifdef __SLEEP_RESET_H__
	Reset_Config(RESET_FLAG_UART4_TX, SET);
	Sleep_Config(SLEEP_FLAG_UART4_TX, SET);
#endif
	Usart_send(UART4, &Usart, buf, count);
}

void UART4_IRQ(void)
{
    /////////// BYTE RECEIVED /////////// BYTE RECEIVED /////////// BYTE RECEIVED
    if(USART_GetITStatus(UART4, USART_IT_RXNE))
    {
        USART_ClearITPendingBit(UART4, USART_IT_IDLE);
#ifdef __SLEEP_RESET_H__
    	Reset_Config(RESET_FLAG_UART4_RX, SET);
    	Sleep_Config(SLEEP_FLAG_UART4_RX, SET);
#endif
    	Usart_receiveByte(UART4, &Usart);
    }

    /////////// RECEIVE COMPLETED /////////// RECEIVE COMPLETED /////////// RECEIVE COMPLETED
    if(USART_GetITStatus(UART4, USART_IT_IDLE)) {
        USART_ClearITPendingBit(UART4, USART_IT_IDLE);
#ifdef __SLEEP_RESET_H__
    	Reset_Config(RESET_FLAG_UART4_RX, RESET);
    	Sleep_Config(SLEEP_FLAG_UART4_RX, RESET);
#endif
        Usart_receiveCompleted(UART4, &Usart);
    }
    /////////// BYTE TRANSMITTING /////////// BYTE TRANSMITTING /////////// BYTE TRANSMITTING
    if(USART_GetITStatus(UART4, USART_IT_TXE)) {
#if (defined(UART4_RS485) && !defined(CHIP_0XX))
        GPIO_SetBits(UART4_DIR_PORT, UART4_DIR_PIN);
#endif
        Usart_transmitByte(UART4, &Usart);
    }
    /////////// TRANSMIT COMPLETE /////////// TRANSMIT COMPLETE
    if (USART_GetITStatus(UART4, USART_IT_TC)) {
        USART_ClearITPendingBit(UART4, USART_IT_TC);
    	Reset_Config(RESET_FLAG_UART4_TX, RESET);
    	Sleep_Config(SLEEP_FLAG_UART4_TX, RESET);
#if (defined(UART4_RS485) && !defined(CHIP_0XX))
        GPIO_ResetBits(UART4_DIR_PORT, UART4_DIR_PIN);
#endif
        Usart_transmitComplete(UART4);
    }
#ifdef CHIP_0XX
    /////////// WAKEUP /////////// WAKEUP
    if (USART_GetITStatus(UART4, USART_IT_WU))
    {
      USART_ClearITPendingBit(UART4, USART_IT_WU);
    }
#endif
}

#endif
