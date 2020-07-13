#ifndef __HARDWARE_H__
#define __HARDWARE_H__

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// область для правки пользователем ///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//* #define LIGHTCONTROLE //в случае использования контроллера освещения
#define MASTER //в случае использования контроллера воды
//#define REGISTRATOR //в случае использования регистратора импульсов

#define GPIO_CPU_LED_PORT				GPIOA			//закомментируйте для отключения
#define GPIO_CPU_LED_PIN				GPIO_Pin_6

#define GPIO_STATUS_LED_PORT			GPIOA			//закомментируйте для отключения
#define GPIO_STATUS_LED_PIN				GPIO_Pin_5

#define GPIO_SIG_LED_PORT				GPIOA			//закомментируйте для отключения
#define GPIO_SIG_LED_PIN				GPIO_Pin_3

//#define GPIO_SIGPWM_LED_PORT			GPIOA			//закомментируйте для отключения
//#define GPIO_SIGPWM_LED_PIN			GPIO_Pin_1

//#define BOOTLOADER						BOOTLOADER		//закомментируйте для отключения
//#define BOOTLOADER_PROGRAM_START_PAGE	14

#define USART1_SPEED					9600
#define USART2_SPEED					9600
#define USART1_RS485					USART1_RS485
#define UART4_SPEED						9600
#define UART5_SPEED						9600
#define UART4_RS485						UART4_RS485

//*Начало Изменено в связи с добалвением 3-го порта 01.07.19
#define USART3_SPEED					9600
#define USART3_RS485					USART3_RS485  //*Нет 485
//*Конец Изменено в связи с добалвением 3-го порта 01.07.19

//#define LSI							LSI				//закомментируйте для использования LSE
#define RTCCLK							32768UL			//частота LSx
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define COUNTERS								16
#define HW_FLASH_POINTER						0x08000000UL
#define HW_FLASH_SIZE							1024UL

#if defined(BOOTLOADER)
	#define HW_FLASH_PSTART_PAGE					BOOTLOADER_PROGRAM_START_PAGE
	#define HW_FLASH_PSTART_ADDRESS					(HW_FLASH_POINTER + HW_FLASH_SIZE * HW_FLASH_PSTART_PAGE)
	#define BOOTLOADER_CONFIG_PAGE_NUMBER           12
	#define BOOTLOADER_CONFIG_PAGE_ADDRESS         	( (uint32_t)(HW_FLASH_POINTER + HW_FLASH_SIZE * BOOTLOADER_CONFIG_PAGE_NUMBER) )

	#define BOOTLOADER_CONFIG_BTLKEY_ADDRESS      	( (uint32_t)(HW_FLASH_POINTER + HW_FLASH_SIZE * BOOTLOADER_CONFIG_PAGE_NUMBER) )
	#define BOOTLOADER_CONFIG_BTLKEY_VALUE         	( *(__IO uint32_t *) BOOTLOADER_CONFIG_BTLKEY_ADDRESS )
	#define BOOTLOADER_KEY_VALUE                    (0xAAAA5555UL)
	//#warning Установите в свойствах линкера стартовый адрес, равный BOOTLOADER_PROGRAM_START_PAGE
#else
	//#warning Bootloader is DISABLED
#endif

#ifdef GPIO_CPU_LED_PORT
	#define GPIO_CPU_LED_ON				do{ GPIO_SetBits(GPIO_CPU_LED_PORT, GPIO_CPU_LED_PIN); } while(0)
	#define GPIO_CPU_LED_OFF			do{ GPIO_ResetBits(GPIO_CPU_LED_PORT, GPIO_CPU_LED_PIN); } while(0)
#else
	#define GPIO_CPU_LED_ON
	#define GPIO_CPU_LED_OFF
#endif

#ifdef GPIO_STATUS_LED_PORT
	#define GPIO_STATUS_LED_ON			do{ GPIO_SetBits(GPIO_STATUS_LED_PORT, GPIO_STATUS_LED_PIN); } while(0)
	#define GPIO_STATUS_LED_OFF			do{ GPIO_ResetBits(GPIO_STATUS_LED_PORT, GPIO_STATUS_LED_PIN); } while(0)
#else
	#define GPIO_STATUS_LED_ON
	#define GPIO_STATUS_LED_OFF
#endif

#ifdef GPIO_SIG_LED_PORT
	#define GPIO_SIG_LED_ON				do{ GPIO_SetBits(GPIO_SIG_LED_PORT, GPIO_SIG_LED_PIN); } while(0)
	#define GPIO_SIG_LED_OFF			do{ GPIO_ResetBits(GPIO_SIG_LED_PORT, GPIO_SIG_LED_PIN); } while(0)
#else
	#define GPIO_SIG_LED_ON
	#define GPIO_SIG_LED_OFF
#endif


//// USART1
#define USART1_CLOCK_SET(x)		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, x);
#define USART1_GPIO_CLOCK_SET(x)		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, x);
//PA10 RX
#define USART1_RX_PORT			GPIOA
#define USART1_RX_PIN			GPIO_Pin_10
#define USART1_RX_MODE_ON		GPIO_Mode_IPU
#define USART1_RX_MODE_OFF		GPIO_Mode_AIN

//PA9 TX
#define USART1_TX_PORT			GPIOA
#define USART1_TX_PIN			GPIO_Pin_9
#define USART1_TX_MODE_ON		GPIO_Mode_AF_PP
#define USART1_TX_MODE_OFF		GPIO_Mode_AIN

#ifdef USART1_RS485
	//PA8 RS485 direction
	#define USART1_DIR_PORT			GPIOA
	#define USART1_DIR_PIN			GPIO_Pin_8
	#define USART1_DIR_MODE_ON		GPIO_Mode_Out_PP
	#define USART1_DIR_MODE_OFF		GPIO_Mode_AIN
#endif

///// USART2
#define USART2_CLOCK_SET(x)			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, x);
#define USART2_GPIO_CLOCK_SET(x)	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, x);

//PA3 RX
#define USART2_RX_PORT			GPIOA
#define USART2_RX_PIN			GPIO_Pin_3
#define USART2_RX_MODE_ON		GPIO_Mode_IPU
#define USART2_RX_MODE_OFF		GPIO_Mode_AIN

//PA2 TX
#define USART2_TX_PORT			GPIOA
#define USART2_TX_PIN			GPIO_Pin_2
#define USART2_TX_MODE_ON		GPIO_Mode_AF_PP
#define USART2_TX_MODE_OFF		GPIO_Mode_AIN

//*Начало Изменено в связи с добалвением 3-го порта 01.07.19
#define USART3_CLOCK_SET(x)				RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3, x);
#define USART3_GPIO_CLOCK_SET(x)		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, x);

//PA10 RX
#define USART3_RX_PORT			GPIOB
#define USART3_RX_PIN			GPIO_Pin_11
#define USART3_RX_MODE_ON		GPIO_Mode_IPU
#define USART3_RX_MODE_OFF		GPIO_Mode_AIN

//PA9 TX
#define USART3_TX_PORT			GPIOB
#define USART3_TX_PIN			GPIO_Pin_10
#define USART3_TX_MODE_ON		GPIO_Mode_AF_PP
#define USART3_TX_MODE_OFF		GPIO_Mode_AIN

//PB13 CTS3<-MODEM_RTS
#define USART3_CTS_PORT			GPIOB
#define USART3_CTS_PIN			GPIO_Pin_13
#define USART3_CTS_MODE_ON		GPIO_Mode_IPU
#define USART3_CTS_MODE_OFF		GPIO_Mode_AIN

//PB13 RTS3->MODEM_CTS3
#define USART3_RTS_PORT			GPIOB
#define USART3_RTS_PIN			GPIO_Pin_14
#define USART3_RTS_MODE_ON		GPIO_Mode_Out_PP
#define USART3_RTS_MODE_OFF		GPIO_Mode_AIN

#ifdef USART3_RS485
	//PA8 RS485 direction
	#define USART3_DIR_PORT			GPIOB
	#define USART3_DIR_PIN			GPIO_Pin_14 //GPIO_Pin_12
	#define USART3_DIR_MODE_ON		GPIO_Mode_Out_PP
	#define USART3_DIR_MODE_OFF		GPIO_Mode_AIN
#endif
//*Конец Изменено в связи с добалвением 3-го порта 01.07.19

//добавление usart4 20200610
#define UART4_CLOCK_SET(x)				RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART4, x);
#define UART4_GPIO_CLOCK_SET(x)			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, x);
//PC11
#define UART4_RX_PORT			GPIOC
#define UART4_RX_PIN			GPIO_Pin_11
#define UART4_RX_MODE_ON		GPIO_Mode_IPU
#define UART4_RX_MODE_OFF		GPIO_Mode_AIN
//PC10
#define UART4_TX_PORT			GPIOC
#define UART4_TX_PIN			GPIO_Pin_10
#define UART4_TX_MODE_ON		GPIO_Mode_AF_PP
#define UART4_TX_MODE_OFF		GPIO_Mode_AIN

#ifdef UART4_RS485
	//PA15 RS485 direction
	#define UART4_DIR_PORT			GPIOA
	#define UART4_DIR_PIN			GPIO_Pin_15
	#define UART4_DIR_MODE_ON		GPIO_Mode_Out_PP
	#define UART4_DIR_MODE_OFF		GPIO_Mode_AIN
#endif

//добавление uart5 20200610
#define UART5_CLOCK_SET(x)				RCC_APB1PeriphClockCmd(RCC_APB1Periph_UART5, x);
#define UART5_GPIO_CLOCK_SET(x)			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, x);
//PD2
#define UART5_RX_PORT			GPIOD
#define UART5_RX_PIN			GPIO_Pin_2
#define UART5_RX_MODE_ON		GPIO_Mode_IPU
#define UART5_RX_MODE_OFF		GPIO_Mode_AIN

//PC12
#define UART5_TX_PORT			GPIOC
#define UART5_TX_PIN			GPIO_Pin_12
#define UART5_TX_MODE_ON		GPIO_Mode_AF_PP
#define UART5_TX_MODE_OFF		GPIO_Mode_AIN
#endif
