#include "main.h"
#ifdef __UART5_H__

PRIVATE tsUartConfig sUartConfig;
PRIVATE Usart_t Usart;

//*PUBLIC inline uint8_t UART5_IsInWork(void)
PUBLIC uint8_t UART5_IsInWork(void)
{
	return CHECK_BIT(Usart.flags, USART_FLAG_INWORK);
}

PUBLIC void UART5_Init(tsUartConfig *cfg)
{
	tsUartConfig *psUartConfig;
	if(cfg == NULL)
	{
		Usart_Config_SetDefault(&sUartConfig, UART5_SPEED);
		psUartConfig = &sUartConfig;
	}
	else
	{
		psUartConfig = cfg;
	}

#ifdef CHIP_0XX
	/* Configure the HSI as USART clock */
	RCC_USARTCLKConfig(RCC_UART5CLK_HSI);
#endif

    /* Enable GPIO clock */
	UART5_GPIO_CLOCK_SET(ENABLE);

    /* Enable USART clock */
	UART5_CLOCK_SET(ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

#ifdef CHIP_0XX
    /* Connect PXx to USARTx_Tx */
    GPIO_PinAFConfig(UART5_TX_PORT, UART5_TX_PSRC, UART5_TX_AFMODE);

    /* Connect PXx to USARTx_Rx */
    GPIO_PinAFConfig(UART5_RX_PORT, UART5_RX_PSRC, UART5_RX_AFMODE);

    /* Configure USART Tx, Rx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_2;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_InitStructure.GPIO_Pin = UART5_TX_PIN;
    GPIO_InitStructure.GPIO_Mode = UART5_TX_MODE_ON;
    GPIO_Init(UART5_TX_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = UART5_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = UART5_RX_MODE_ON;
    GPIO_Init(UART5_RX_PORT, &GPIO_InitStructure);

	#ifdef UART5_RS485
		/* Connect PXx to USARTx_DE */
		GPIO_PinAFConfig(UART5_DE_PORT, UART5_DE_PSRC, UART5_DE_AFMODE);

		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Pin = UART5_DE_PIN;
		GPIO_InitStructure.GPIO_Mode = UART5_DE_MODE_ON;
		GPIO_Init(UART5_DE_PORT, &GPIO_InitStructure);

		USART_DECmd(UART5, ENABLE);
		USART_DEPolarityConfig(UART5, USART_DEPolarity_High);
		//USART_SetDEAssertionTime(UART5, 4);
		//USART_SetDEDeassertionTime(UART5, 4);
	#endif
#else
    GPIO_InitStructure.GPIO_Pin   = UART5_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode  = UART5_TX_MODE_ON;
    GPIO_Init(UART5_TX_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = UART5_RX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode  = UART5_RX_MODE_ON;
    GPIO_Init(UART5_RX_PORT, &GPIO_InitStructure);
	#ifdef UART5_RS485
		GPIO_InitStructure.GPIO_Pin   = UART5_DIR_PIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
		GPIO_InitStructure.GPIO_Mode  = UART5_DIR_MODE_ON;
		GPIO_Init(UART5_DIR_PORT, &GPIO_InitStructure);

		GPIO_ResetBits(UART5_DIR_PORT, UART5_DIR_PIN);
	#endif

	#ifdef __SLEEP_RESET_H__
		Sleep_Config(SLEEP_FLAG_UART5_RX, SET);
	#endif
#endif

    Usart_Init(UART5, &Usart, psUartConfig, UART5_IRQn);
}

//*PUBLIC inline uint8_t UART5_Subscribe(ACTION_DATA(receive))
PUBLIC  uint8_t UART5_Subscribe(ACTION_DATA(receive))
{
	if(Usart.dataReceive == NULL)
	{
		Usart.dataReceive = receive;
		return 1;
	}
	return 0;
}

//PUBLIC inline void UART5_send(uint8_t *buf, uint16_t count)
PUBLIC void UART5_send(uint8_t *buf, uint16_t count)
{
#ifdef __SLEEP_RESET_H__
	Reset_Config(RESET_FLAG_UART5_TX, SET);
	Sleep_Config(SLEEP_FLAG_UART5_TX, SET);
#endif
	Usart_send(UART5, &Usart, buf, count);
}

void UART5_IRQ(void)
{
	//GPIO_CPU_LED_ON;
    /////////// BYTE RECEIVED /////////// BYTE RECEIVED /////////// BYTE RECEIVED
    if(USART_GetITStatus(UART5, USART_IT_RXNE))
    {
        USART_ClearITPendingBit(UART5, USART_IT_IDLE);
#ifdef __SLEEP_RESET_H__
    	Reset_Config(RESET_FLAG_UART5_RX, SET);
    	Sleep_Config(SLEEP_FLAG_UART5_RX, SET);
#endif

		gFromUsart = UART5_NUMBER;
    	Usart_receiveByte(UART5, &Usart);
    }

    /////////// RECEIVE COMPLETED /////////// RECEIVE COMPLETED /////////// RECEIVE COMPLETED
    if(USART_GetITStatus(UART5, USART_IT_IDLE)) {
        USART_ClearITPendingBit(UART5, USART_IT_IDLE);
#ifdef __SLEEP_RESET_H__
    	Reset_Config(RESET_FLAG_UART5_RX, RESET);
    	Sleep_Config(SLEEP_FLAG_UART5_RX, RESET);
#endif
        Usart_receiveCompleted(UART5, &Usart);
    }
    /////////// BYTE TRANSMITTING /////////// BYTE TRANSMITTING /////////// BYTE TRANSMITTING
    if(USART_GetITStatus(UART5, USART_IT_TXE)) {
#if (defined(UART5_RS485) && !defined(CHIP_0XX))
        GPIO_SetBits(UART5_DIR_PORT, UART5_DIR_PIN);
#endif
        Usart_transmitByte(UART5, &Usart);
    }
    /////////// TRANSMIT COMPLETE /////////// TRANSMIT COMPLETE
    if (USART_GetITStatus(UART5, USART_IT_TC)) {
        USART_ClearITPendingBit(UART5, USART_IT_TC);
#ifdef __SLEEP_RESET_H__
    	Reset_Config(RESET_FLAG_UART5_TX, RESET);
    	Sleep_Config(SLEEP_FLAG_UART5_TX, RESET);
#endif
#if (defined(UART5_RS485) && !defined(CHIP_0XX))
        GPIO_ResetBits(UART5_DIR_PORT, UART5_DIR_PIN);
#endif
        Usart_transmitComplete(UART5);
    }
#ifdef CHIP_0XX
    /////////// WAKEUP /////////// WAKEUP
    if (USART_GetITStatus(UART5, USART_IT_WU))
    {
      USART_ClearITPendingBit(UART5, USART_IT_WU);
    }
#endif
}

#endif

