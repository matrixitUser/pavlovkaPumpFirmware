//#include "main.h"
#include "stdafx.h"
#include "masterTimer.h"

void TIM2_Init(void)
{
	RCC_APB1PeriphClockCmd(	RCC_APB1Periph_TIM2,ENABLE);
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	//setting timer 3 interrupt to 1hz ((800000/8000)*1000)s
		TIM_TimeBaseStructure.TIM_Prescaler = 8000 ;   //1000Hz
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;// TIM_CounterMode_Up;
		TIM_TimeBaseStructure.TIM_Period = 1000-1;
		TIM_TimeBaseStructure.TIM_ClockDivision = 0;
		TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
		/* TIM IT enable */
		TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

		/* TIM3 enable counter */
		TIM_Cmd(TIM2, DISABLE);


	NVIC_InitTypeDef NVIC_InitStructure;

	//enable tim3 irq
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_PRIORITY_PREEMPTIVE(NVIC_PRIORITY_TIMER);
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = IRQ_PRIORITY_PREEMPTIVE(NVIC_PRIORITY_TIMER);
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);

	NVIC_EnableIRQ(TIM2_IRQn);
}
//timer 3 interrupt
void TIM2_IRQHandler(void)
{

	//if interrupt happens the do this
	if(TIM_GetITStatus(TIM2, TIM_IT_Update) != RESET)
	{
		//clear interrupt and start counting again to get precise freq
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
		TIM_Cmd(TIM2, DISABLE);
		g_timeout2 = 0;
	}
}

void TIM3_Init(void)
{
	RCC_APB1PeriphClockCmd(	RCC_APB1Periph_TIM3,ENABLE);
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	//setting timer 3 interrupt to 1hz ((2400000/24000)*1000)s
		TIM_TimeBaseStructure.TIM_Prescaler = 8000 ;
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Down;// TIM_CounterMode_Up;
		TIM_TimeBaseStructure.TIM_Period = 1000-1;
		TIM_TimeBaseStructure.TIM_ClockDivision = 0;
		TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
		/* TIM IT enable */
		TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);

		/* TIM3 enable counter */
		TIM_Cmd(TIM3, DISABLE);


	NVIC_InitTypeDef NVIC_InitStructure;

	//enable tim3 irq
	NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_PRIORITY_PREEMPTIVE(NVIC_PRIORITY_TIMER);
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = IRQ_PRIORITY_PREEMPTIVE(NVIC_PRIORITY_TIMER);
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);

	//NVIC_SetPriority (TIM3_IRQn, 4);

	NVIC_EnableIRQ(TIM3_IRQn);
	//NVIC_SetPriority (USART1_IRQn, 4);
}
//timer 3 interrupt
void TIM3_IRQHandler(void)
{

	//if interrupt happens the do this
	if(TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		if(secBeforeOn < TIME_UPP_ACTIVE)
		{
			g_forseToError=1;
			secBeforeOn++;
		}
		else
		{
			g_forseToError=0;
			secBeforeOn = 0;
			TIM_Cmd(TIM3, DISABLE);
		}
		//clear interrupt and start counting again to get precise freq
		TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
}


void TIM4_Init(void)
{
	RCC_APB1PeriphClockCmd(	RCC_APB1Periph_TIM4,ENABLE);
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	//setting timer 3 interrupt to 1hz ((2400000/24000)*1000)s
		TIM_TimeBaseStructure.TIM_Prescaler = 8000 ;
		TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;// TIM_CounterMode_Up;
		TIM_TimeBaseStructure.TIM_Period = 1000-1;
		TIM_TimeBaseStructure.TIM_ClockDivision = 0;
		TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);
		/* TIM IT enable */
		TIM_ITConfig(TIM4, TIM_IT_Update, ENABLE);

		/* TIM3 enable counter */
		TIM_Cmd(TIM4, DISABLE);


	NVIC_InitTypeDef NVIC_InitStructure;

	//enable tim3 irq
	NVIC_InitStructure.NVIC_IRQChannel = TIM4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_PRIORITY_PREEMPTIVE(NVIC_PRIORITY_TIMER);
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = IRQ_PRIORITY_PREEMPTIVE(NVIC_PRIORITY_TIMER);
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure);

	//NVIC_SetPriority (TIM3_IRQn, 4);

	NVIC_EnableIRQ(TIM4_IRQn);
	//NVIC_SetPriority (USART1_IRQn, 4);
}

//timer 4 interrupt
void TIM4_IRQHandler(void)
{
	//if interrupt happens the do this
	if(TIM_GetITStatus(TIM4, TIM_IT_Update) != RESET)
	{
		//clear interrupt and start counting again to get precise freq
		TIM_ClearITPendingBit(TIM4, TIM_IT_Update);
		g_secound++;
		gWatchDog++;if(gWatchDog > 15){    IWDG_ReloadCounter();  gWatchDog =0;}
	}
}

void TIM7_Init(void)
{
	RCC_APB1PeriphClockCmd(	RCC_APB1Periph_TIM7,ENABLE);
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	//setting timer 7 interrupt to 1hz ((2400000/24000)*1000)s
	TIM_TimeBaseStructure.TIM_Prescaler = 8000 ;
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;// TIM_CounterMode_Up;
	TIM_TimeBaseStructure.TIM_Period = 5000-1;
	TIM_TimeBaseStructure.TIM_ClockDivision = 0;
	TIM_TimeBaseInit(TIM7, &TIM_TimeBaseStructure);
	/* TIM IT enable */
	TIM_ITConfig(TIM7, TIM_IT_Update, ENABLE);

	/* TIM7 enable counter */

	NVIC_InitTypeDef NVIC_InitStructure;

	//enable tim3 irq
	NVIC_InitStructure.NVIC_IRQChannel = TIM7_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = IRQ_PRIORITY_PREEMPTIVE(NVIC_PRIORITY_TIMER);
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = IRQ_PRIORITY_PREEMPTIVE(NVIC_PRIORITY_TIMER);
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

	NVIC_Init(&NVIC_InitStructure); //запуск TIM7_IRQHandler
	NVIC_EnableIRQ(TIM7_IRQn);
}

//timer 4 interrupt
void TIM7_IRQHandler(void){
	//if interrupt happens the do this
	if(TIM_GetITStatus(TIM7, TIM_IT_Update) != RESET)
	{
		//clear interrupt and start counting again to get precise freq
		TIM_ClearITPendingBit(TIM7, TIM_IT_Update);

		printf("%d TIM7_IRQHandler\n\r", RTC_GetCounter());
		TIM_Cmd(TIM7, DISABLE);
		if(g_tsCb.cb != NULL)
		{
			//printf("TIM4_IRQHandler if(g_tsCb.cb != NULL) \n\r");
			Callback_Save(&g_tsCb);
			g_tsCb.cb = NULL;
		}
	}
}
