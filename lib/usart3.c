#include "main.h"
//#ifdef __USART3_H__

PRIVATE tsUartConfig sUartConfig;
PRIVATE Usart_t Usart;

//*PUBLIC inline uint8_t Usart3_IsInWork(void)
PUBLIC uint8_t Usart3_IsInWork(void)
{
	return CHECK_BIT(Usart.flags, USART_FLAG_INWORK);
}

PUBLIC void Usart3_Init(tsUartConfig *cfg)
{

	tsUartConfig *psUartConfig;
	if(cfg == NULL)
	{
		Usart_Config_SetDefault(&sUartConfig, USART3_SPEED);
		psUartConfig = &sUartConfig;
	}
	else
	{
		psUartConfig = cfg;
	}

#ifdef CHIP_0XX
	/* Configure the HSI as USART clock */
	RCC_USARTCLKConfig(RCC_USART3CLK_HSI);
#endif

    /* Enable GPIO clock */
	USART3_GPIO_CLOCK_SET(ENABLE);

    /* Enable USART3 clock */
	USART3_CLOCK_SET(ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;

#ifdef CHIP_0XX
    /* Connect PXx to USARTx_Tx */
    GPIO_PinAFConfig(USART3_TX_PORT, USART3_TX_PSRC, USART3_TX_AFMODE);

    /* Connect PXx to USARTx_Rx */
    GPIO_PinAFConfig(USART3_RX_PORT, USART3_RX_PSRC, USART3_RX_AFMODE);

    /* Configure USART Tx, Rx as alternate function push-pull */
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_Level_2;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;

    GPIO_InitStructure.GPIO_Pin = USART3_TX_PIN;
    GPIO_InitStructure.GPIO_Mode = USART3_TX_MODE_ON;
    GPIO_Init(USART3_TX_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = USART3_RX_PIN;
    GPIO_InitStructure.GPIO_Mode = USART3_RX_MODE_ON;
    GPIO_Init(USART3_RX_PORT, &GPIO_InitStructure);

	#ifdef USART3_RS485
		/* Connect PXx to USARTx_DE */
		GPIO_PinAFConfig(USART3_DE_PORT, USART3_DE_PSRC, USART3_DE_AFMODE);

		GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
		GPIO_InitStructure.GPIO_Pin = USART3_DE_PIN;
		GPIO_InitStructure.GPIO_Mode = USART3_DE_MODE_ON;
		GPIO_Init(USART3_DE_PORT, &GPIO_InitStructure);

		USART_DECmd(USART3, ENABLE);
		USART_DEPolarityConfig(USART3, USART_DEPolarity_High);
		//USART_SetDEAssertionTime(USART1, 4);
		//USART_SetDEDeassertionTime(USART1, 4);
	#endif
#else
    GPIO_InitStructure.GPIO_Pin   = USART3_TX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode  = USART3_TX_MODE_ON;
    GPIO_Init(USART3_TX_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = USART3_RX_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode  = USART3_RX_MODE_ON;
    GPIO_Init(USART3_RX_PORT, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin   = USART3_CTS_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode  = USART3_CTS_MODE_ON;
    GPIO_Init(USART3_CTS_PORT, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin   = USART3_RTS_PIN;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
    GPIO_InitStructure.GPIO_Mode  = USART3_RTS_MODE_ON;
    GPIO_Init(USART3_RTS_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(USART3_RTS_PORT,USART3_RTS_PIN);

	#ifdef USART3_RS485
		GPIO_InitStructure.GPIO_Pin   = USART3_DIR_PIN;
		GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
		GPIO_InitStructure.GPIO_Mode  = USART3_DIR_MODE_ON;
		GPIO_Init(USART3_DIR_PORT, &GPIO_InitStructure);

		GPIO_ResetBits(USART3_DIR_PORT, USART3_DIR_PIN);
	#endif

	#ifdef __SLEEP_RESET_H__
		Sleep_Config(SLEEP_FLAG_USART3_RX, SET);
	#endif
#endif

    Usart_Init(USART3, &Usart, psUartConfig, USART3_IRQn);
}

//*PUBLIC inline uint8_t Usart1_Subscribe(ACTION_DATA(receive))
PUBLIC  uint8_t Usart3_Subscribe(ACTION_DATA(receive))
{
	if(Usart.dataReceive == NULL)
	{
		Usart.dataReceive = receive;

		return 1;
	}
	return 0;
}

//PUBLIC inline void Usart1_send(uint8_t *buf, uint16_t count)
PUBLIC void Usart3_send(uint8_t *buf, uint16_t count)
{
	//led5blink();
#ifdef __SLEEP_RESET_H__
	Reset_Config(RESET_FLAG_USART3_TX, SET);
	Sleep_Config(SLEEP_FLAG_USART3_TX, SET);
#endif
	Usart_send(USART3, &Usart, buf, count);
}

void USART3_IRQ(void)
{

	//GPIO_CPU_LED_ON;
    /////////// BYTE RECEIVED /////////// BYTE RECEIVED /////////// BYTE RECEIVED
    if(USART_GetITStatus(USART3, USART_IT_RXNE))
    {
        USART_ClearITPendingBit(USART3, USART_IT_IDLE);
#ifdef __SLEEP_RESET_H__
    	Reset_Config(RESET_FLAG_USART3_RX, SET);
    	Sleep_Config(SLEEP_FLAG_USART3_RX, SET);
#endif

		gFromUsart = USART3_NUMBER;
    	Usart_receiveByte(USART3, &Usart);
    	//sendTo1();
    }
    /////////// RECEIVE COMPLETED /////////// RECEIVE COMPLETED /////////// RECEIVE COMPLETED
    if(USART_GetITStatus(USART3, USART_IT_IDLE)) {
        USART_ClearITPendingBit(USART3, USART_IT_IDLE);
#ifdef __SLEEP_RESET_H__
    	Reset_Config(RESET_FLAG_USART3_RX, RESET);
    	Sleep_Config(SLEEP_FLAG_USART3_RX, RESET);
#endif
        Usart_receiveCompleted(USART3, &Usart);
    }
    /////////// BYTE TRANSMITTING /////////// BYTE TRANSMITTING /////////// BYTE TRANSMITTING
    if(USART_GetITStatus(USART3, USART_IT_TXE)) {
#if (defined(USART3_RS485) && !defined(CHIP_0XX))
        GPIO_SetBits(USART3_DIR_PORT, USART3_DIR_PIN);
#endif
        Usart_transmitByte(USART3, &Usart);
    }
    /////////// TRANSMIT COMPLETE /////////// TRANSMIT COMPLETE
    if (USART_GetITStatus(USART3, USART_IT_TC)) {


        USART_ClearITPendingBit(USART3, USART_IT_TC);
#ifdef __SLEEP_RESET_H__
    	Reset_Config(RESET_FLAG_USART3_TX, RESET);
    	Sleep_Config(SLEEP_FLAG_USART3_TX, RESET);
#endif
#if (defined(USART3_RS485) && !defined(CHIP_0XX))
        GPIO_ResetBits(USART3_DIR_PORT, USART3_DIR_PIN);
#endif
        Usart_transmitComplete(USART3);
    }
#ifdef CHIP_0XX
    /////////// WAKEUP /////////// WAKEUP
    if (USART_GetITStatus(USART3, USART_IT_WU))
    {
      USART_ClearITPendingBit(USART3, USART_IT_WU);
    }
#endif
}

//#endif
