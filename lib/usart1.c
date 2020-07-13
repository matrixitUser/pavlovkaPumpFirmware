#include "main.h"
#ifdef __USART1_H__

PRIVATE tsUartConfig sUartConfig;
PRIVATE Usart_t Usart;

//*PUBLIC inline uint8_t Usart1_IsInWork(void)
PUBLIC uint8_t Usart1_IsInWork(void)
{
	return CHECK_BIT(Usart.flags, USART_FLAG_INWORK);
}

PUBLIC void Usart1_Init(tsUartConfig *cfg)
{
	tsUartConfig *psUartConfig;
	if(cfg == NULL)
	{
		Usart_Config_SetDefault(&sUartConfig, USART1_SPEED);
		psUartConfig = &sUartConfig;
	}
	else
	{
		psUartConfig = cfg;
	}

#ifdef CHIP_0XX
	/* Configure the HSI as USART clock */
	RCC_USARTCLKConfig(RCC_USART1CLK_HSI);
#endif

    /* Enable GPIO clock */
	USART1_GPIO_CLOCK_SET(ENABLE);

    /* Enable USART clock */
	USART1_CLOCK_SET(ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

#ifdef CHIP_0XX
    /* Connect PXx to USARTx_Tx */
    GPIO_PinAFConfig(USART1_TX_PORT, USART1_TX_PSRC, USART1_TX_AFMODE);

    /* Connect PXx to USARTx_Rx */
    GPIO_PinAFConfig(USART1_RX_PORT, USART1_RX_PSRC, USART1_RX_AFMODE);

    /* Configure USART Tx, Rx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_2;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_InitStructure.GPIO_Pin = USART1_TX_PIN;
    GPIO_InitStructure.GPIO_Mode = USART1_TX_MODE_ON;
    GPIO_Init(USART1_TX_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = USART1_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = USART1_RX_MODE_ON;
    GPIO_Init(USART1_RX_PORT, &GPIO_InitStructure);

	#ifdef USART1_RS485
		/* Connect PXx to USARTx_DE */
		GPIO_PinAFConfig(USART1_DE_PORT, USART1_DE_PSRC, USART1_DE_AFMODE);

		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Pin = USART1_DE_PIN;
		GPIO_InitStructure.GPIO_Mode = USART1_DE_MODE_ON;
		GPIO_Init(USART1_DE_PORT, &GPIO_InitStructure);

		USART_DECmd(USART1, ENABLE);
		USART_DEPolarityConfig(USART1, USART_DEPolarity_High);
		//USART_SetDEAssertionTime(USART1, 4);
		//USART_SetDEDeassertionTime(USART1, 4);
	#endif
#else
    GPIO_InitStructure.GPIO_Pin   = USART1_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode  = USART1_TX_MODE_ON;
    GPIO_Init(USART1_TX_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = USART1_RX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode  = USART1_RX_MODE_ON;
    GPIO_Init(USART1_RX_PORT, &GPIO_InitStructure);
	#ifdef USART1_RS485
		GPIO_InitStructure.GPIO_Pin   = USART1_DIR_PIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
		GPIO_InitStructure.GPIO_Mode  = USART1_DIR_MODE_ON;
		GPIO_Init(USART1_DIR_PORT, &GPIO_InitStructure);

		GPIO_ResetBits(USART1_DIR_PORT, USART1_DIR_PIN);
	#endif

	#ifdef __SLEEP_RESET_H__
		Sleep_Config(SLEEP_FLAG_USART1_RX, SET);
	#endif
#endif

    Usart_Init(USART1, &Usart, psUartConfig, USART1_IRQn);
}

//*PUBLIC inline uint8_t Usart1_Subscribe(ACTION_DATA(receive))
PUBLIC  uint8_t Usart1_Subscribe(ACTION_DATA(receive))
{
	if(Usart.dataReceive == NULL)
	{
		Usart.dataReceive = receive;
		return 1;
	}
	return 0;
}

//PUBLIC inline void Usart1_send(uint8_t *buf, uint16_t count)
PUBLIC void Usart1_send(uint8_t *buf, uint16_t count)
{
#ifdef __SLEEP_RESET_H__
	Reset_Config(RESET_FLAG_USART1_TX, SET);
	Sleep_Config(SLEEP_FLAG_USART1_TX, SET);
#endif
	Usart_send(USART1, &Usart, buf, count);
}

void USART1_IRQ(void)
{
	//GPIO_CPU_LED_ON;
    /////////// BYTE RECEIVED /////////// BYTE RECEIVED /////////// BYTE RECEIVED
    if(USART_GetITStatus(USART1, USART_IT_RXNE))
    {
        USART_ClearITPendingBit(USART1, USART_IT_IDLE);
#ifdef __SLEEP_RESET_H__
    	Reset_Config(RESET_FLAG_USART1_RX, SET);
    	Sleep_Config(SLEEP_FLAG_USART1_RX, SET);
#endif
    	Usart_receiveByte(USART1, &Usart);
    }

    /////////// RECEIVE COMPLETED /////////// RECEIVE COMPLETED /////////// RECEIVE COMPLETED
    if(USART_GetITStatus(USART1, USART_IT_IDLE)) {
        USART_ClearITPendingBit(USART1, USART_IT_IDLE);
#ifdef __SLEEP_RESET_H__
    	Reset_Config(RESET_FLAG_USART1_RX, RESET);
    	Sleep_Config(SLEEP_FLAG_USART1_RX, RESET);
#endif
        Usart_receiveCompleted(USART1, &Usart);
    }
    /////////// BYTE TRANSMITTING /////////// BYTE TRANSMITTING /////////// BYTE TRANSMITTING
    if(USART_GetITStatus(USART1, USART_IT_TXE)) {
#if (defined(USART1_RS485) && !defined(CHIP_0XX))
        GPIO_SetBits(USART1_DIR_PORT, USART1_DIR_PIN);
#endif
        Usart_transmitByte(USART1, &Usart);
    }
    /////////// TRANSMIT COMPLETE /////////// TRANSMIT COMPLETE
    if (USART_GetITStatus(USART1, USART_IT_TC)) {
        USART_ClearITPendingBit(USART1, USART_IT_TC);
#ifdef __SLEEP_RESET_H__
    	Reset_Config(RESET_FLAG_USART1_TX, RESET);
    	Sleep_Config(SLEEP_FLAG_USART1_TX, RESET);
#endif
#if (defined(USART1_RS485) && !defined(CHIP_0XX))
        GPIO_ResetBits(USART1_DIR_PORT, USART1_DIR_PIN);
#endif
        Usart_transmitComplete(USART1);
    }
#ifdef CHIP_0XX
    /////////// WAKEUP /////////// WAKEUP
    if (USART_GetITStatus(USART1, USART_IT_WU))
    {
      USART_ClearITPendingBit(USART1, USART_IT_WU);
    }
#endif
}

#endif

