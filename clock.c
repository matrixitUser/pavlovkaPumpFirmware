#include "clock.h"

PUBLIC void Clock_Init(void)
{
	// SYSCLK = HSI без PLL => PCLKx = HCLK = SYSCLK = 8МГц
	RCC_DeInit();
	RCC_HSICmd(ENABLE);
	RCC_SYSCLKConfig(RCC_SYSCLKSource_HSI);
	RCC_HSEConfig(RCC_HSE_OFF);
	RCC_HCLKConfig(RCC_SYSCLK_Div1);//HCLK = SYSCLK
	RCC_PCLK2Config(RCC_HCLK_Div1);//PCLK2 = HCLK/2
	RCC_PCLK1Config(RCC_HCLK_Div1);//PCLK1 = HCLK/2
	RCC_PLLCmd(DISABLE);//Disable PLL

	// включение периферии
    RCC_APB2PeriphClockCmd
    (
    	//enable
    	RCC_APB2Periph_GPIOA |
    	RCC_APB2Periph_GPIOB |
    	RCC_APB2Periph_GPIOC |
    	RCC_APB2Periph_GPIOD |
    	RCC_APB2Periph_AFIO |
    	RCC_APB2Periph_TIM15 |
    	RCC_APB2Periph_TIM16 |
    	RCC_APB2Periph_USART1,
    	ENABLE
    );

    RCC_APB1PeriphClockCmd
	(
		RCC_APB1Periph_PWR |
		RCC_APB1Periph_BKP |
    	RCC_APB1Periph_TIM2 |
    	RCC_APB1Periph_TIM7,
    	ENABLE
	);
}
