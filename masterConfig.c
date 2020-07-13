#include "masterConfig.h"

PRIVATE u8 CheckConfig(mstrConfig *psConfig);
PRIVATE void ReadConfig(void);
PRIVATE void WriteConfig(void);

////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//external->current
PUBLIC u8 Master_Config_Set(mstrConfig *psConfig)
{
	if(!CheckConfig(psConfig)) return 0;
	if(memcmp(psConfig, &g_mConfig, sizeof(mstrConfig)) != 0)
	{
		memcpy(&g_mConfig, psConfig, sizeof(mstrConfig));
		WriteConfig();
	}
	return 1;
}

PUBLIC u8 Master_Config_Init(void)
{
	ReadConfig();

	Usart_Config_SetDefault(&g_sConfig.sUart1, USART1_SPEED);
	Usart_Config_SetDefault(&g_sConfig.sUart3, USART3_SPEED);

	if(CheckConfig(&g_mConfig))
	{

		return 1;
	}

	Master_Config_Reset();
	return 0;
}

PUBLIC void Master_Config_Reset(void)
{
	g_mConfig.NA = (uint8_t)240;
	g_mConfig.UPP_NA = (uint8_t)47;
	g_mConfig.WLS_NA = (uint8_t)16;
	g_mConfig.UPPlastRespTime = 0;
	g_mConfig.WLSlastRespTime = 0;
	WriteConfig();
	}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

PRIVATE u8 CheckConfig(mstrConfig *psConfig)
{
	if(psConfig->NA > 239)
	{
		return 0;
	}
	/*
	if(psConfig->UPP_NA > 239)
	{
		return 0;
	}

	if(psConfig->WLS_NA > 239)
	{
		return 0;
	}
	 */
	return 1;
}

PRIVATE void ReadConfig(void)
{
	Flash_Read1(CONFIG_PAGE, (uint8_t *)&g_mConfig, sizeof(mstrConfig));
}

PRIVATE void WriteConfig(void)
{
	Flash_Write1(CONFIG_PAGE, (uint8_t *)&g_mConfig, sizeof(mstrConfig), 1); //TODO изменить адрес (page)
}
