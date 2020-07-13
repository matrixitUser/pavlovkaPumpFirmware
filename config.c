#include "config.h"

#ifdef __CONFIG_H__

//PRIVATE u8 CheckConfig(tsConfig *psConfig);
//PRIVATE void ReadConfig(void);
PRIVATE void WriteConfig(void);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//external->current
PUBLIC u8 Config_Set(tsConfig *psConfig, ResetType_e eReset)
{
	printf("-------------Config_Set \n\r");
	if(!CheckConfig(psConfig)) return 0;
	g_writeConfig = 1;
	if(memcmp(psConfig, &g_sConfig, sizeof(tsConfig)) != 0)
	{
		memcpy(&g_sConfig, psConfig, sizeof(tsConfig));
		WriteConfig();
		g_writeConfig = 2;

		if(eReset == RESET_SOFT)
		{
			Reset_Soft();
		}
		else if(eReset == RESET_HARD)
		{

		}
	}
	return 1;
}

PUBLIC u8 Config_Init(void)
{
	printf("-------------Config_Init \n\r");
	ReadConfig();

	if(CheckConfig(&g_sConfig))
	{
		return 1;
	}

	Config_Reset();
	return 0;
}


#ifdef PAVLOVKA
PUBLIC void Config_Reset(void)
{
	uint8_t i;
	memset(&g_sConfig, 0, sizeof(tsConfig));

	g_sConfig.u16FlashVer = 			FLASH_HELLO;
	g_sConfig.u8NetworkAddress = 		NETWORK_ADDRESS == 0? 1 : (NETWORK_ADDRESS > 247? 247 : NETWORK_ADDRESS); //Сетевой адрес
	g_sConfig.u8Mode =					MODE_MASTER;// MODE_SLAVE;	//  //mode slave //

	g_sConfig.UPP_NA_LEFT = 			47;
	g_sConfig.UPP_NA_RIGHT = 			48;
	g_sConfig.WLS_NA = 					16;
	g_sConfig.PumpTimeIdleMax = 		30; //Максимальное время простоя, мин
	g_sConfig.WlsServerMode = 			WLSMODE_MC_ONLY;
	g_sConfig.FirstUPP =				FIRST_UPP_LEFT;

	g_sConfig.isUsedSecondUpp =			SECOND_UPP_IS_USED;

	Usart_Config_SetDefault(&g_sConfig.sUart1, USART1_SPEED);								// usart config reset
	Usart_Config_SetDefault(&g_sConfig.sUart2, USART2_SPEED);
	Usart_Config_SetDefault(&g_sConfig.sUart3, USART3_SPEED);
	Usart_Config_SetDefault(&g_sConfig.sUart4, UART4_SPEED);
	Usart_Config_SetDefault(&g_sConfig.sUart5, UART5_SPEED);

	WriteConfig();
}
#endif
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PUBLIC u8 CheckConfigNew(tsConfig *psConfig)
{
	if(psConfig->u8NetworkAddress != NETWORK_ADDRESS)
	{
		return 0;
	}
	if((psConfig->isUsedSecondUpp != SECOND_UPP_IS_USED) && (psConfig->isUsedSecondUpp != SECOND_UPP_NOT_USED))
	{
		return 0;
	}
	return 1;
}


PUBLIC u8 CheckConfig(tsConfig *psConfig)
{

	if(psConfig->u16FlashVer != FLASH_HELLO)
	{
		g_writeConfig = 3;
		return 0;
	}

	if((psConfig->u8NetworkAddress == 0) || (psConfig->u8NetworkAddress > 247))
	{
		g_writeConfig = 4;
		return 0;
	}

	if((psConfig->u8Mode != MODE_SLAVE) && (psConfig->u8Mode != MODE_MASTER))
	{
		g_writeConfig = 5;
		return 0;
	}


	if(psConfig->u8IsRtcError > 1)
	{
		g_writeConfig = 6;
		return 0;
	}

	if(!Usart_Config_Check(&psConfig->sUart1))								// usart config check
	{

		return 0;
	}

	return 1;
}

PUBLIC void ReadConfig(void)
{
	Flash_Read1(CONFIG_PAGE, (uint8_t *)&g_sConfig, sizeof(tsConfig));
	printf("-------------ReadConfig \n\r");
}

PUBLIC void WriteConfig(void)
{
	Flash_Write1(CONFIG_PAGE, (uint8_t *)&g_sConfig, sizeof(tsConfig), 1);
	printf("-------------WriteConfig \n\r");
}

#endif
