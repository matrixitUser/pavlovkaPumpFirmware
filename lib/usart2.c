#include "main.h"
#ifdef __USART2_H__

PRIVATE tsUartConfig sUartConfig;
PRIVATE Usart_t Usart;

PUBLIC uint8_t Usart2_IsInWork(void)
{
	return CHECK_BIT(Usart.flags, USART_FLAG_INWORK);
}

PUBLIC void Usart2_Init(tsUartConfig *cfg)
{
	tsUartConfig *psUartConfig;
	if(cfg == NULL)
	{
		Usart_Config_SetDefault(&sUartConfig, USART2_SPEED);
		psUartConfig = &sUartConfig;
	}
	else
	{
		psUartConfig = cfg;
	}

    /* Enable GPIO clock */
	USART2_GPIO_CLOCK_SET(ENABLE);

    /* Enable USART clock */
	USART2_CLOCK_SET(ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

#ifdef CHIP_0XX
    /* Connect PXx to USARTx_Tx */
    GPIO_PinAFConfig(USART2_TX_PORT, USART2_TX_PSRC, USART2_TX_AFMODE);

    /* Connect PXx to USARTx_Rx */
    GPIO_PinAFConfig(USART2_RX_PORT, USART2_RX_PSRC, USART2_RX_AFMODE);

    /* Configure USART Tx, Rx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_2;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_InitStructure.GPIO_Pin = USART2_TX_PIN;
    GPIO_InitStructure.GPIO_Mode = USART2_TX_MODE_ON;
    GPIO_Init(USART2_TX_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = USART2_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = USART2_RX_MODE_ON;
    GPIO_Init(USART2_RX_PORT, &GPIO_InitStructure);

#ifdef USART2_RS485
    /* Connect PXx to USARTx_DE */
    GPIO_PinAFConfig(USART2_DE_PORT, USART2_DE_PSRC, USART2_DE_AFMODE);

    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Pin = USART2_DE_PIN;
    GPIO_InitStructure.GPIO_Mode = USART2_DE_MODE_ON;
    GPIO_Init(USART2_DE_PORT, &GPIO_InitStructure);

    USART_DECmd(USART2, ENABLE);
    USART_DEPolarityConfig(USART2, USART_DEPolarity_High);
    //USART_SetDEAssertionTime(USART2, 4);
    //USART_SetDEDeassertionTime(USART2, 4);
#endif
#else
    GPIO_InitStructure.GPIO_Pin   = USART2_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode  = USART2_TX_MODE_ON;
    GPIO_Init(USART2_TX_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = USART2_RX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode  = USART2_RX_MODE_ON;
    GPIO_Init(USART2_RX_PORT, &GPIO_InitStructure);
#ifdef USART2_RS485
    GPIO_InitStructure.GPIO_Pin   = USART2_DIR_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode  = USART2_DIR_MODE_ON;
    GPIO_Init(USART2_DIR_PORT, &GPIO_InitStructure);

	GPIO_ResetBits(USART2_DIR_PORT, USART2_DIR_PIN);
#endif

	Sleep_Config(SLEEP_FLAG_USART2_RX, SET);
#endif

    Usart_Init(USART2, &Usart, psUartConfig, USART2_IRQn);
}

PUBLIC uint8_t Usart2_Subscribe(ACTION_DATA(receive))
{
	if(Usart.dataReceive == NULL)
	{
		Usart.dataReceive = receive;
		return 1;
	}
	return 0;
}

//PUBLIC inline void Usart2_send(uint8_t *buf, uint16_t count)
PUBLIC void Usart2_send(uint8_t *buf, uint16_t count)
{
#ifdef __SLEEP_RESET_H__
	Reset_Config(RESET_FLAG_USART2_TX, SET);
	Sleep_Config(SLEEP_FLAG_USART2_TX, SET);
#endif
	Usart_send(USART2, &Usart, buf, count);
}

void USART2_IRQ(void)
{
	GPIO_CPU_LED_ON;
    /////////// BYTE RECEIVED /////////// BYTE RECEIVED /////////// BYTE RECEIVED
    if(USART_GetITStatus(USART2, USART_IT_RXNE))
    {
        USART_ClearITPendingBit(USART2, USART_IT_IDLE);
    	Reset_Config(RESET_FLAG_USART2_RX, SET);
    	Sleep_Config(SLEEP_FLAG_USART2_RX, SET);
    	Usart_receiveByte(USART2, &Usart);
    }

    /////////// RECEIVE COMPLETED /////////// RECEIVE COMPLETED /////////// RECEIVE COMPLETED
    if(USART_GetITStatus(USART2, USART_IT_IDLE)) {
        USART_ClearITPendingBit(USART2, USART_IT_IDLE);
    	Reset_Config(RESET_FLAG_USART2_RX, RESET);
    	Sleep_Config(SLEEP_FLAG_USART2_RX, RESET);
        Usart_receiveCompleted(USART2, &Usart);
    }
    /////////// BYTE TRANSMITTING /////////// BYTE TRANSMITTING /////////// BYTE TRANSMITTING
    if(USART_GetITStatus(USART2, USART_IT_TXE)) {
#if (defined(USART2_RS485) && !defined(CHIP_0XX))
        GPIO_SetBits(USART2_DIR_PORT, USART2_DIR_PIN);
#endif
        Usart_transmitByte(USART2, &Usart);
    }
    /////////// TRANSMIT COMPLETE /////////// TRANSMIT COMPLETE
    if (USART_GetITStatus(USART2, USART_IT_TC)) {
        USART_ClearITPendingBit(USART2, USART_IT_TC);
    	Reset_Config(RESET_FLAG_USART2_TX, RESET);
    	Sleep_Config(SLEEP_FLAG_USART2_TX, RESET);
#if (defined(USART2_RS485) && !defined(CHIP_0XX))
        GPIO_ResetBits(USART2_DIR_PORT, USART2_DIR_PIN);
#endif
        Usart_transmitComplete(USART2);
    }
#ifdef CHIP_0XX
    /////////// WAKEUP /////////// WAKEUP
    if (USART_GetITStatus(USART2, USART_IT_WU))
    {
      USART_ClearITPendingBit(USART2, USART_IT_WU);
    }
#endif
}

#endif
