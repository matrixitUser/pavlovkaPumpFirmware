#ifndef __IRQ_H__
#define __IRQ_H__

///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////
#define IRQ_PREEMPTIVE				IRQ_PREEMPTIVE
#define NVIC_PRIORITY_TIMER		0		//0~15
#define NVIC_PRIORITY_RTC		1		//0~15
#define NVIC_PRIORITY_USART		2		//0~15
///////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

#define SYSTEM_TIMER_IRQ		SysTick_Handler

#define USART1_IRQ				USART1_IRQHandler
#define USART2_IRQ				USART2_IRQHandler
#define USART3_IRQ				USART3_IRQHandler
#define UART4_IRQ				UART4_IRQHandler
#define UART5_IRQ				UART5_IRQHandler

#if (FLAGS & FLAGS_UART2_DEBUG) > 0
#define USART2_IRQ				USART2_IRQHandler
#endif

//#define TIMER_IRQ				TIM6_DAC_IRQHandler
//#define TIMER2_IRQ				TIM7_IRQHandler

#define TMRFILTER_IRQ			TIM7_IRQHandler
#define TMRRTCCHECK_IRQ			TIM1_BRK_TIM15_IRQHandler
#define TMR_STATUS_LED_IRQ		TIM1_UP_TIM16_IRQHandler

//#define TIMER_SIGPWM			TIM2_IRQHandler

#define CHANNEL0_IRQ			EXTI0_IRQHandler
#define CHANNEL1_IRQ			EXTI1_IRQHandler
#define CHANNEL2_IRQ			EXTI2_IRQHandler
#define CHANNEL3_IRQ			EXTI3_IRQHandler
#define CHANNEL4_IRQ			EXTI4_IRQHandler
#define CHANNEL9_5_IRQ			EXTI9_5_IRQHandler
#define CHANNEL15_10_IRQ		EXTI15_10_IRQHandler

#define RTCALARM_IRQ			RTCAlarm_IRQHandler

#define ADC1_IRQ				ADC1_IRQHandler



#if defined(IRQ_PREEMPTIVE)
#define IRQ_GROUP						NVIC_PriorityGroup_4
#define IRQ_PRIORITY_PREEMPTIVE(x)		x
#define IRQ_PRIORITY_NONPREEMPTIVE(x)	0
#else
#define IRQ_GROUP				NVIC_PriorityGroup_0		//no preemption
#define IRQ_PRIORITY_PREEMPTIVE(x)		0
#define IRQ_PRIORITY_NONPREEMPTIVE(x)	x
#endif
#define IRQ_INIT(group)			NVIC_PriorityGroupConfig(group)


#endif
