#include "frame_modbus.h"

//#if 1// defined(__FRAMEMMODBUS_H__)
/*
#include "config.h"
//#include "channel_v2.h"
#include "counter.h"
#include "hourly.h"
#include "abnormal.h"
#include "rtcalarm.h"
#include "crc.h"
#include "usart1.h"
#include "usart3.h"
#include "sleep_reset.h"
#include "adc1.h"*/

#define V_25		18630193UL 	//(1.41V / 3.1V) * 4096 		x10000
#define AVG_SLOPE	568			//(4.3mV/*C / 3.1V) * 4096		x100

u8* TmpData;
u8 rqstIdle = 1;
//PRIVATE uint8_t fullNA = 0;

PRIVATE void FrameError(uint8_t fun, uint8_t answer)
{
	uint8_t data[1] = {answer};
	FrameModbus_Send(fun|0x80, data, 1);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#define TIME_CORRECTION_MAX_SEC		120
#define MAX_REG_BYTES				180

// запись данных различного размера в массив байт
PRIVATE void _DataGathering(uint8_t *pTo, void *pFrom, uint8_t type, uint8_t arraySize)
{
	uint8_t size = type & MODBUS_REG_SIZE_MASK;
	uint8_t j;
	for(j = 0; j < arraySize; j++, pTo += size)
	{
		if(pFrom == NULL)
		{
			memset(pTo, 0, size);
		}
		else
		{
			uint32_t temp = 0;
			switch(type)
			{
				case MODBUS_REGR_TYPE_CONSTANT_MB_UINT32:
					temp = (uint32_t)pFrom;
					memrcpy_u32(pTo, temp);
					break;

				case MODBUS_REGR_TYPE_CONSTANT_UINT32:
					temp = (uint32_t)pFrom;
					memcpy_u32(pTo, temp);
					break;

				case MODBUS_REGR_TYPE_CONSTANT_MB_UINT16:
					temp = (uint32_t)pFrom;
					memrcpy_u16(pTo, (uint16_t)temp);
					break;

				case MODBUS_REGR_TYPE_CONSTANT_UINT16:
					temp = (uint32_t)pFrom;
					memcpy_u16(pTo, (uint16_t)temp);
					break;

				case MODBUS_REGR_TYPE_CONSTANT_BYTE:
					temp = (uint32_t)pFrom;
					*pTo = (uint8_t)temp;
					break;

				case MODBUS_REGR_TYPE_LINK_MB_UINT32:
					memrcpy_u32(pTo, *((uint32_t *)pFrom + j));
					break;

				case MODBUS_REGR_TYPE_LINK_UINT32:
					memcpy_u32(pTo, *((uint32_t *)pFrom + j));
					break;

				case MODBUS_REGR_TYPE_LINK_MB_UINT16:
					memrcpy_u16(pTo, *((uint16_t *)pFrom + j));
					break;

				case MODBUS_REGR_TYPE_LINK_UINT16:
					memcpy_u16(pTo, *((uint16_t *)pFrom + j));
					break;

				case MODBUS_REGR_TYPE_LINK_BYTE:
					*pTo = *((uint8_t *)pFrom + j);
					break;

				case MODBUS_REGR_TYPE_COMPUTE_UINT32:
				case MODBUS_REGR_TYPE_COMPUTE_MB_UINT32:
					if(pFrom == MODBUS_REGR_COMPUTE_TIMESTAMP)
					{
						temp = Override_RTC_GetCounter();
					}
					pFrom = &temp;
					if(CHECK_BITS(type, MODBUS_REG_IS_MB))
					{
						memrcpy_u32(pTo, *(uint32_t *)pFrom);
					}
					else
					{
						memcpy_u32(pTo, *(uint32_t *)pFrom);
					}
					break;
					if(pFrom == MODBUS_REGR_COMPUTE_ADC_TEMP)
					{
						Adc1_Start();
						uint32_t rawTemp = Adc1_getRaw(16);
						uint32_t rawTemp4k = rawTemp * 10000;
						int16_t tempC2k;
						if(V_25 >= rawTemp4k)
						{
							tempC2k = 2500 + ( ((uint32_t)(V_25 - rawTemp4k)) / AVG_SLOPE);
						}
						else
						{
							tempC2k = 2500 - ( ((int32_t)(rawTemp4k - V_25)) / AVG_SLOPE);
						}
						temp = tempC2k;
					}
					else if(pFrom == MODBUS_REGR_COMPUTE_ADC_BAT)
					{
						uint16_t rawBat = Adc1_getRaw(17);
						uint16_t batV3K = (rawBat == 0)? 0 : ((1200UL << 12) / rawBat);
						temp = batV3K;
					}

					pFrom = &temp;
					if(CHECK_BITS(type, MODBUS_REG_IS_MB))
					{
						memrcpy_u16(pTo, *(uint16_t *)pFrom);
					}
					else
					{
						memcpy_u16(pTo, *(uint16_t *)pFrom);
					}
					break;

				case MODBUS_REGR_TYPE_COMPUTE_BYTE:
					if(pFrom == MODBUS_REGR_COMPUTE_CFGMODE)
					{
						temp = g_sConfig.u8Mode;
					}

					*pTo = (uint8_t)temp;
					break;
				default:
					break;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// массив -> u8, uint16_t или u32
PRIVATE void _DataWrite(void *pTo, uint8_t *pFrom, uint8_t type, uint8_t arraySize)
{
	uint8_t i;
	for(i = 0; i < arraySize; i++)
	{
		if(pFrom == NULL)
		{
			memset(pTo, 0, type & MODBUS_REG_SIZE_MASK);
		}
		else
		{
			switch(type)
			{
			case MODBUS_REGW_TYPE_MB_UINT32:
				*((uint32_t *)pTo + i) = memrget_u32(pFrom + (i << 2));
				break;

			case MODBUS_REGW_TYPE_UINT32:
				*((uint32_t *)pTo + i) = memget_u32(pFrom + (i << 2));
				break;

			case MODBUS_REGW_TYPE_MB_UINT16:
				*((uint16_t *)pTo + i) = memrget_u16(pFrom + (i << 1));
				break;

			case MODBUS_REGW_TYPE_UINT16:
				*((uint16_t *)pTo + i) = memget_u16(pFrom + (i << 1));
				break;

			case MODBUS_REGW_TYPE_BYTE:
				*((uint8_t *)pTo + i) = *(pFrom + i);
				break;

			default:
				break;
			}
		}
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//frame(0) modbus функция
/// frame  -первый байт после адреса, len - длина frame (без CRC)
PRIVATE void FrameProcess(u8 hdl, u32 len)
{
	if(!rqstIdle) return;
	rqstIdle = 0;
	uint8_t* frame = TmpData;
	printf("----FrameProcess----g_sConfig.u8NetworkAddress == %d \n\r",g_sConfig.u8NetworkAddress);

	if(!CheckConfigNew(&g_sConfig)) ReadConfig();
	uint8_t isObject=0;
	long int u32Ts=0;
	uint8_t* _arrTmp;
	tsConfig* tsTmp = &g_sConfig;
	tsConfig sMyNewCfg;
	//struct tm* currenTime;
	if(len==0){
		rqstIdle = 1;
		return;
	}

	uint8_t ret[1 + MAX_REG_BYTES];
	memset(ret, 0, 1 + MAX_REG_BYTES);

	uint8_t* mfd = frame+1;  //  старший байт запрашиваемого регистра
	uint16_t mfl = len-1;    //

	switch(frame[0])
	{
		case MODBUS_FUNCTION_READ_HOLDING_REGISTERS:
		{
			if(mfl==4)
			{
				uint16_t startReg = ((u16)mfd[0] << 8) | mfd[1];
				uint16_t numOfReg = ((u16)mfd[2] << 8) | mfd[3];

				if((0xFFFF - startReg) < numOfReg)
				{
					FrameError(frame[0], MODBUS_ERROR_ADDRESS_NOT_EXIST);
					rqstIdle = 1;
					return;
				}

				if(numOfReg <= MAX_REG_BYTES)
				{
					ret[0] = numOfReg;
					uint8_t i;
					uint8_t modbusError = MODBUS_ERROR_NO_ERROR;

					for(i = 0; (i < numOfReg) && (modbusError == MODBUS_ERROR_NO_ERROR); )
					{
						uint8_t type = MODBUS_REGR_TYPE_LINK_BYTE;
						uint8_t arraySize = 1;
						void *pFrom = NULL;

						uint8_t *pTo = &ret[1 + i];
						uint16_t addr = startReg + i;

						// маппинг
						if(addr < sizeof(tsConfig))   //внутри длины config
						{
							type = MODBUS_REGR_TYPE_LINK_BYTE;
							pFrom = &((uint8_t *)&g_sConfig)[addr];
						}
						else
						{
							switch(addr)
							{
							case REGHLD_TIMESTAMP:
							case REGHLD_TIMESTAMP_COPY:
								type = MODBUS_REGR_TYPE_COMPUTE_MB_UINT32;
								pFrom = MODBUS_REGR_COMPUTE_TIMESTAMP;
								break;

							default:
								break;
							}
						}
						if((type & MODBUS_REG_SIZE_MASK) == 0)
						{
							modbusError = MODBUS_ERROR_EXCEPTION;
						}

						// исполнение по данным

						if(modbusError == MODBUS_ERROR_NO_ERROR)
						{
							i += (type & MODBUS_REG_SIZE_MASK);
							if(i <= numOfReg)
							{
								_DataGathering(pTo, pFrom, type, arraySize);
							}
							else
							{
								modbusError = MODBUS_ERROR_MEMORY_ALIGN_ERROR;
							}
						}
					}

					if(modbusError == MODBUS_ERROR_NO_ERROR)
					{
						FrameModbus_Send(frame[0], ret, 1 + numOfReg);
					}
					else
					{
						FrameError(frame[0], modbusError);
					}

				}
				else
				{
					FrameError(frame[0], MODBUS_ERROR_INSUFFICIENT_MEMORY);//ошибка: слишком много регистров запрошено
				}
			}
			else
			{
				FrameError(frame[0], MODBUS_ERROR_INVALID_VALUE);
			}
			break;
		}

		case MODBUS_USER_FUNCTION_GET_EQUIPMENT_INFO:
		{
			g_accessLevel = ACCESS_LEVEL_MASTER;//TODO
			g_timeout = 0;
			uint8_t offsetMB = 0;

			uint8_t bytes[4];

			offsetMB = 1;
			uint32_t timeFromServer = ( ((uint32_t)frame[offsetMB+19]) << 24) | (((uint32_t)frame[offsetMB+18]) << 16) | (((uint32_t)frame[offsetMB+17]) << 8) | (((uint32_t)frame[offsetMB+16]) << 0);
			if(RTC_GetCounter() < 1568909764)
			{
				Rtc_SetTime(timeFromServer);
			}

			if(g_accessLevel >= ACCESS_LEVEL_MASTER)
			{
				if(mfl > 13) //0x4A,  startReg= 01,00,numOfReg= 00,count= 0E,  FF,08,07,00,F0,95,87,5B,01,40,DC,87,5B,00,F5,69
				{
					//Установить GUID  //Id объекта а БД
					uint8_t  objectId[16];
					uint8_t isGuid = 0;
					int i;
					for(i = 0;i < 16; i++)
					{
						objectId[i] = frame[offsetMB+i];
						if(frame[offsetMB+i] != 0xFF ) isGuid = 1;  //хотя бы один не FF
					}
					if(isGuid) memcpy(g_objectId,objectId,sizeof(objectId));

					// Данные WLS, полученные опросом башни сервером
					if(frame[offsetMB+20] != 0xFF) //WLS из сервера, 1 или 2 (использовать
					{
						g_WLSserverMode  = frame[offsetMB+20];  // WLS из сервера:  0- не использутся; 1- используется, если не удалось опросить
																//					2- только из сервера (позволяет управлять стоп/пуск двигателя
						g_sConfig.WlsServerMode =frame[offsetMB+20];
						printf("g_WLSserverMode=%d  g_sConfig.WlsServerMode = %d \n\r",g_WLSserverMode, g_sConfig.WlsServerMode);
						if(g_sConfig.WlsServerMode > WLSMODE_MC_ONLY)
						{
							g_WLSfromserver  = frame[offsetMB+21];  //байт WLS
							//Высота из сервера

							union
							{
								float float_variable;
								uint8_t outBytes[4];
							} heightServer;

							heightServer.outBytes[0] = frame[offsetMB+22];
							heightServer.outBytes[1] = frame[offsetMB+23];
							heightServer.outBytes[2] = frame[offsetMB+24];
							heightServer.outBytes[3] = frame[offsetMB+25];
							g_WLS.height = heightServer.float_variable;
							g_WLS.lastRespTime = (uint32_t)((frame[offsetMB+26]<<0) | (frame[offsetMB+27]<<8) | (frame[offsetMB+28]<<16) | (frame[offsetMB+29]<<24));
						}
					}

					CMD50(&ret);

					FrameModbus_Send(frame[0], ret, 87);
				}
				else
				{
					FrameError(frame[0], MODBUS_ERROR_INVALID_VALUE);
				}
			}
			else
			{
				FrameError(frame[0], MODBUS_ERROR_NO_ACCESS);
			}
			break;
		}

		case MODBUS_USER_FUNCTION_CONFIG_SEND:
		{
			if(mfl == 3)
			{
				 _arrTmp = ret;
				*((tsConfig*)_arrTmp) = g_sConfig;
				FrameModbus_Send(frame[0], ret,(sizeof(tsConfig) & 0xFF)); //TODO вычислить размер и проверить
			}
			else
			{
				FrameError(frame[0], MODBUS_ERROR_INVALID_VALUE);
			}
			break;
		}

		case MODBUS_USER_FUNCTION_CONFIG_RECEIVE:
		{
			tsTmp = (tsConfig*)(&frame[1]);
			sMyNewCfg = *tsTmp;
			g_writeConfig = 0;
			Config_Set(&sMyNewCfg, RESET_SOFT);
			ret[0]=0;ret[1]=g_writeConfig;
			FrameModbus_Send(frame[0], ret,2);
			break;
		}

		case MODBUS_FUNCTION_WRITE_HOLDING_REGISTERS:
		{
			if(mfl >= 5) //Длина фрейма без команды
			{
				uint16_t startReg = ((u16)mfd[0] << 8) | mfd[1];  //mfd = frame+1 2 байта номера регистра
				uint16_t numOfReg = ((u16)mfd[2] << 8) | mfd[3];  // кол-во регистров

				if((0xFFFF - startReg) < numOfReg)
				{
					FrameError(frame[0], MODBUS_ERROR_ADDRESS_NOT_EXIST);	//вне диапазона
				}
				else if(numOfReg > MAX_REG_BYTES)
				{
					FrameError(frame[0], MODBUS_ERROR_INSUFFICIENT_MEMORY);	//слишком много байт для записи
				}
				else if(numOfReg != (mfl - 4))  // 4 == 2 байта номера регистра + 2 байта кол-ва регистров
				{
					FrameError(frame[0], MODBUS_ERROR_EXCEPTION);			//несовпадение количества байт и длины
				}
				else
				{
					uint8_t ret[4];
					ret[0] = mfd[0];
					ret[1] = mfd[1];
					uint8_t i;
					uint8_t modbusError = MODBUS_ERROR_NO_ERROR;

					uint32_t u32BtlKey = 0;
					uint32_t u32fRegwCommit = 0;
					uint32_t u32Ts = TIMESTAMP_UNDEFINED;

					uint32_t u32CounterPulse[COUNTERS];
					uint32_t u32CounterValue[COUNTERS];

					for(i = 0; i < COUNTERS; i++)
					{
						u32CounterPulse[i] = WORD_UNDEFINED;
						u32CounterValue[i] = WORD_UNDEFINED;
					}

					uint8_t pPassword[24];
					tsConfig sNewCfg;
					memcpy(&sNewCfg, &g_sConfig, sizeof(tsConfig));

					for(i = 0; (i < numOfReg) && (modbusError == MODBUS_ERROR_NO_ERROR);)
					{
						uint8_t type = MODBUS_REGW_TYPE_BYTE;
						uint8_t arraySize = 1;
						void *pTo = NULL;

						uint8_t *pFrom = &mfd[4 + i];
						uint16_t addr = startReg + i;

						switch(addr)
						{
							case REGHLD_PASSWORD_SESSION_0:    //400 is PASSWORD
								type = MODBUS_REGW_TYPE_BYTE;
								pTo = (void *)pPassword;
								arraySize = 24;
								SETBITS(u32fRegwCommit, MODBUS_REGW_COMMIT_PASSWORD);
								break;

							default:
								if(g_accessLevel >= ACCESS_LEVEL_MASTER)
								{
									if(addr < 0x0100)   // в регистрах до 0x0100 хранится CONFIG
									{
										type = MODBUS_REGW_TYPE_BYTE;
										pTo = (void *)(((uint8_t *)&sNewCfg) + addr);
										SETBITS(u32fRegwCommit, MODBUS_REGW_COMMIT_CONFIG);
									}
									else
									{
										switch(addr)
										{
											case REGHLD_TIMESTAMP:      //корректировка времени
											case REGHLD_TIMESTAMP_COPY:
												{
													type = MODBUS_REGW_TYPE_MB_UINT32;
													pTo = (void *)&u32Ts;
													SETBITS(u32fRegwCommit, MODBUS_REGW_COMMIT_TIMESTAMP);
												}
												break;

	#ifdef __COUNTER_H__
											case REGHLD_COUNTER_PULSE_1:
											case REGHLD_COUNTER_PULSE_2:
											case REGHLD_COUNTER_PULSE_3:
											case REGHLD_COUNTER_PULSE_4:
											case REGHLD_COUNTER_PULSE_5:
											case REGHLD_COUNTER_PULSE_6:
											case REGHLD_COUNTER_PULSE_7:
											case REGHLD_COUNTER_PULSE_8:
											case REGHLD_COUNTER_PULSE_9:
											case REGHLD_COUNTER_PULSE_10:
											case REGHLD_COUNTER_PULSE_11:
											case REGHLD_COUNTER_PULSE_12:
											case REGHLD_COUNTER_PULSE_13:
											case REGHLD_COUNTER_PULSE_14:
											case REGHLD_COUNTER_PULSE_15:
											case REGHLD_COUNTER_PULSE_16:
												{
													uint8_t n = ((addr - REGHLD_COUNTER_PULSE_1) >> 2) & 0xF;
													type = MODBUS_REGW_TYPE_MB_UINT32;
													pTo = (void *) &u32CounterPulse[n];
												}
												break;

											case REGHLD_COUNTER_VALUE_1:
											case REGHLD_COUNTER_VALUE_2:
											case REGHLD_COUNTER_VALUE_3:
											case REGHLD_COUNTER_VALUE_4:
											case REGHLD_COUNTER_VALUE_5:
											case REGHLD_COUNTER_VALUE_6:
											case REGHLD_COUNTER_VALUE_7:
											case REGHLD_COUNTER_VALUE_8:
											case REGHLD_COUNTER_VALUE_9:
											case REGHLD_COUNTER_VALUE_10:
											case REGHLD_COUNTER_VALUE_11:
											case REGHLD_COUNTER_VALUE_12:
											case REGHLD_COUNTER_VALUE_13:
											case REGHLD_COUNTER_VALUE_14:
											case REGHLD_COUNTER_VALUE_15:
											case REGHLD_COUNTER_VALUE_16:
												{
													uint8_t n = ((addr - REGHLD_COUNTER_VALUE_1) >> 2) & 0xF;
													type = MODBUS_REGW_TYPE_MB_UINT32;
													pTo = (void *)&u32CounterValue[n];
												}
												break;
	#endif
											case REGHLD_BOOTLOADER_KEY:
												if(g_accessLevel >= ACCESS_LEVEL_MASTER)
												{
													type = MODBUS_REGW_TYPE_MB_UINT32;
													pTo = (void *)&u32BtlKey;
													SETBITS(u32fRegwCommit, MODBUS_REGW_COMMIT_BOOTLOADER_KEY);
												}
												else
												{
													modbusError = MODBUS_ERROR_NO_ACCESS;
												}
												break;
										}
									}
									break;
								}
								else
								{
									modbusError = MODBUS_ERROR_NO_ACCESS;
								}
								break;
						} // end SWITCH

						if((type & MODBUS_REG_SIZE_MASK) == 0)
						{
							modbusError = MODBUS_ERROR_EXCEPTION;
						}

						// исполнение по данным

						if(modbusError == MODBUS_ERROR_NO_ERROR)
						{
							i += ((type & MODBUS_REG_SIZE_MASK) * arraySize);
							if(i <= numOfReg)
							{
								_DataWrite(pTo, pFrom, type, arraySize);
							}
							else
							{
								modbusError = MODBUS_ERROR_MEMORY_ALIGN_ERROR;
							}
						}
					}

					if(modbusError == MODBUS_ERROR_NO_ERROR)
					{
						// в рабочем режиме можно записать только регистр пароля
						if(CHECK_BITS(u32fRegwCommit, MODBUS_REGW_COMMIT_PASSWORD))
						{
							char apass[PASSWORD_LENGTH+1];

							//get master password
							memset(apass, 0, sizeof(apass));
							strcpy(apass, PASSWORD_MASTER);
							if(!memcmp(apass, pPassword, PASSWORD_LENGTH))
							{
								g_accessLevel = ACCESS_LEVEL_MASTER;
							}
							else
							{
								memset(apass, 0, sizeof(apass));
								strcpy(apass, PASSWORD_SERVICE);
								if(!memcmp(apass, pPassword, PASSWORD_LENGTH))
								{
									g_accessLevel = ACCESS_LEVEL_SERVICE;
								}
								else
								{
									g_accessLevel = ACCESS_LEVEL_NONE;
								}
							}
						}

						// Установка счетных регистров или показателей
	#ifdef __COUNTER_H__
						uint8_t i;
						for(i = 0; i < 16; i++)
						{
							if(u32CounterPulse[i] != WORD_UNDEFINED)
							{
								Counter_SetPCnt(i, u32CounterPulse[i]);
							}
							if(u32CounterValue[i] != WORD_UNDEFINED)
							{
								Counter_SetFromValue(i, u32CounterValue[i]);
							}
							sNewCfg.asChannel[i].u8IsError = 0;
							SETBITS(u32fRegwCommit, MODBUS_REGW_COMMIT_CONFIG);
						}
	#endif
						if(CHECK_BITS(u32fRegwCommit, MODBUS_REGW_COMMIT_TIMESTAMP))
						{
							Rtc_SetTime(u32Ts); //Здесь корректируется время по correctTime
							CLRBIT(g_errorCode,ERR_TIMER);// из lightControle.c
							//RTC_SetCounter(u32Ts);
							sNewCfg.u8IsRtcError = 0;
							SETBITS(u32fRegwCommit, MODBUS_REGW_COMMIT_CONFIG);
						}

						if(CHECK_BITS(u32fRegwCommit, MODBUS_REGW_COMMIT_CONFIG))
						{
							Config_Set(&sNewCfg, RESET_SOFT);
						}

	#if defined(BOOTLOADER)
						//ret[3]=0xFE;
						if(CHECK_BITS(u32fRegwCommit, MODBUS_REGW_COMMIT_BOOTLOADER_KEY))
						{
							FLASH_Unlock();
							FLASH_ErasePage(BOOTLOADER_CONFIG_BTLKEY_ADDRESS);
							FLASH_ProgramWord(BOOTLOADER_CONFIG_BTLKEY_ADDRESS, u32BtlKey);
							FLASH_Lock();
							Reset_Soft();
							//ret[3]=0xFF;
						}
	#endif


						memrcpy_u16(&ret[2], numOfReg);
						FrameModbus_Send(frame[0], ret, 4);
					}
					else
					{
						FrameError(frame[0], modbusError);
					}
				}
			}
			else
			{
				FrameError(frame[0], MODBUS_ERROR_INVALID_VALUE);
			}
			break;
		}

		case MODBUS_SWITCH_CONTROLLER_MODE:
		{
			if(g_sConfig.u8Mode == MODE_MASTER)
			{
				ServerCall(MODBUS_SWITCH_CONTROLLER_MODE);
			}
			else
			{
				ret[0]=0; //  не может поменять mode, тк slave
				ret[1]=g_sConfig.u8Mode;
				FrameModbus_Send(frame[0], ret,2);
			}
			break;
		}
	}


	if(!CheckConfigNew(&g_sConfig)) ReadConfig();
	rqstIdle = 1;
}

void ServerCall(u8 hdl)
{
	tsCallback tsCB = {cb: ServerCallbackParse, hdl: hdl};
	Callback_Save(&tsCB);
}

void ServerCallbackParse(u8 hdl, u32 param) // hdl === fun
{
	switch(hdl)
	{
		case MODBUS_SWITCH_CONTROLLER_MODE:
			SwitchCtrlMode();
			return;
	}
}

void SwitchCtrlMode()
{
	uint8_t ret1[4];
	ArrFillDATANULL(&ret1);
	u8 ret[2];
	if(!Talking_Send(TALKING_SWITCH_MASTER, ret1)) ret[0] = 0;
	else if(g_sConfig.u8Mode == MODE_MASTER) ret[0] = 0;
	else ret[0] = 1;
	ret[1] = g_sConfig.u8Mode;
	FrameModbus_Send(MODBUS_SWITCH_CONTROLLER_MODE, ret, sizeof(ret));
}


PUBLIC void CMD50(uint8_t *ret){

	//uint8_t ret[1 + MAX_REG_BYTES];
	//memset(ret, 0, 1 + MAX_REG_BYTES);

	long int u32Ts = RTC_GetCounter();

	uint8_t bytes[4];
	memcpy(bytes,&u32Ts,4);
	ret[0] = bytes[0];
	ret[1] = bytes[1];
	ret[2] = bytes[2];
	ret[3] = bytes[3];

	//Сетевые адреса активных устройств

	ret[4] = g_sConfig.u8Mode;
	ret[5] = 0xFF; //reserved for state other controller
	ret[6] = g_UPPstruct->NA;
	ret[7] = g_SecondUPP->NA;

	ret[8] = g_sConfig.isUsedSecondUpp;

	ret[9] = g_WLS.NA;
	//////////////////////////////////////////Контроллер
	ret[10] = (
			(GPIO_ReadInputDataBit(GPIO_PORT_LEFT_STATUS_WORK, GPIO_PIN_LEFT_STATUS_WORK)<<1)
			); //IN status
	ret[11] = (
			(GPIO_ReadOutputDataBit(GPIO_PORT_STOP, GPIO_PIN_STOP)<<3)
			| (GPIO_ReadOutputDataBit(GPIO_PORT_START, GPIO_PIN_START)<<3)
			); //OUT status
	//ошибки контроллера
	ret[12] = g_errorCode>>8;
	ret[13] = g_errorCode&0xFF;
	//события контроллера
	ret[14] = g_eventCode>>8;
	ret[15] = g_eventCode&0xFF; //11

	ret[16] = gTransferAllow; // Разрешение на трансфер

	// тип и версия !!! Передавать FLASH из config
	//ret[13] = (TYPE_PAVLOVKA<<2) | (VERSION<<0);
	//ret[14] = sizeof(tsConfig); // Размер конфига
	ret[17] = g_sConfig.u16FlashVer>>8;
	ret[18] = g_sConfig.u16FlashVer & 0xFF; //14


	//uint8_t leftUpp = g_leftUppActivity;
	//uint8_t rightUpp = g_rightUppActivity;
	//ret[15] = leftUpp<<1 | rightUpp<<0; // Состояния переключателя выбора УПП

	/////////////////////////////////////УПП

	//Время последнего опроса УПП
	memcpy(bytes,&g_UPPstruct->lastRespTime,4);
	ret[19] = bytes[0];
	ret[20] = bytes[1];
	ret[21] = bytes[2];
	ret[22] = bytes[3];

	ret[23] = g_UPPstruct->status>>8;
	ret[24] = g_UPPstruct->status&0xFF;

	memcpy(bytes,&g_SecondUPP->lastRespTime,4);
	ret[25] = bytes[0];
	ret[26] = bytes[1];
	ret[27] = bytes[2];
	ret[28] = bytes[3];

	ret[29] = g_SecondUPP->status>>8;
	ret[30] = g_SecondUPP->status&0xFF;

	//////////////////////////////////WLS

	//Время последнего опроса WLS
	memcpy(bytes,&g_WLS.lastRespTime,4);
	ret[31] = bytes[0];
	ret[32] = bytes[1];
	ret[33] = bytes[2];
	ret[34] = bytes[3];

	//Байт состояния WLS
	ret[35] = g_WLS.status>>8;
	ret[36] = g_WLS.status&0xFF;

	//Датчик давления
	//reserved
	ret[37] = 0xFF; //28
	ret[38] = 0xFF;
	ret[39] = 0xFF;
	ret[40] = 0xFF;

	//Байт состояния Датчика давления
	union
	{
		float float_variable;
		uint8_t outBytes[4];
	} hh;
	hh.float_variable = g_WLS.height;
	ret[41]= hh.outBytes[0];ret[42]= hh.outBytes[1];ret[43]= hh.outBytes[2];ret[44]= hh.outBytes[3];

	//3.4. Analog input telegram (из описания регистров modbus УПП) как есть, то есть старшим байтом вперед
	//0200 - 020ch по 2 байта
	ret[45] = (uint8_t) (g_UPPstruct->motorCurrent >>8);//36
	ret[46] = (uint8_t) (g_UPPstruct->motorCurrent & 0xFF);

	ret[47] = (uint8_t) (g_UPPstruct->thermalLoad >>8);
	ret[48] = (uint8_t) (g_UPPstruct->thermalLoad & 0xFF);

	ret[49] = (uint8_t) (g_UPPstruct->currentPhase1 >>8);
	ret[50] = (uint8_t) (g_UPPstruct->currentPhase1 & 0xFF);

	ret[51] = (uint8_t) (g_UPPstruct->currentPhase2 >>8);
	ret[52] = (uint8_t) (g_UPPstruct->currentPhase2 & 0xFF);

	ret[53] = (uint8_t) (g_UPPstruct->currentPhase3 >>8);
	ret[54] = (uint8_t) (g_UPPstruct->currentPhase3 & 0xFF);

	ret[55] = (uint8_t) (g_UPPstruct->currentPhaseMax >>8);
	ret[56] = (uint8_t) (g_UPPstruct->currentPhaseMax & 0xFF);

	ret[57] = (uint8_t) (g_UPPstruct->frequency >>8);
	ret[58] = (uint8_t) (g_UPPstruct->frequency & 0xFF);

	ret[59] = (uint8_t) (g_UPPstruct->power >>8);
	ret[60] = (uint8_t) (g_UPPstruct->power & 0xFF);

	ret[61] = (uint8_t) (g_UPPstruct->voltage >>8);
	ret[62] = (uint8_t) (g_UPPstruct->voltage & 0xFF);

	ret[63] = (uint8_t) (g_UPPstruct->startCount >>8);
	ret[64] = (uint8_t) (g_UPPstruct->startCount & 0xFF);

	ret[65] = (uint8_t) (g_UPPstruct->runtimeHours >>8);
	ret[66] = (uint8_t) (g_UPPstruct->runtimeHours & 0xFF);

	ret[67] = (uint8_t) (g_UPPstruct->modbusError >>8);
	ret[68] = (uint8_t) (g_UPPstruct->modbusError & 0xFF);

	ret[69] = (uint8_t) (g_UPPstruct->modbusToglle >>8);
	ret[70] = (uint8_t) (g_UPPstruct->modbusToglle & 0xFF);

	memcpy(bytes,&g_UPPstruct->lastStartTime,4);
	ret[71] = bytes[0];    										//62
	ret[72] = bytes[1];
	ret[73] = bytes[2];
	ret[74] = bytes[3];

	memcpy(bytes,&g_UPPstruct->lastStopTime,4);
	ret[75] = bytes[0];
	ret[76] = bytes[1];
	ret[77] = bytes[2];
	ret[78] = bytes[3];

	memcpy(bytes,&g_SecondUPP->lastStartTime,4);
	ret[79] = bytes[0];
	ret[80] = bytes[1];
	ret[81] = bytes[2];
	ret[82] = bytes[3];

	memcpy(bytes,&g_SecondUPP->lastStopTime,4);
	ret[83] = bytes[0];
	ret[84] = bytes[1];
	ret[85] = bytes[2];
	ret[86] = bytes[3];
}


PUBLIC void FrameModbus_Init(void){

}


PUBLIC void FrameModbus_DataProcess(uint8_t *data, uint16_t len)
{
	printf(" u8Mode=%d--FrameModbus_DataProcess--u8NetworkAddress == %d \n\r",g_sConfig.u8Mode ,g_sConfig.u8NetworkAddress);
	if(len < 4) return;

	uint8_t head = 1;   //Первый байт после адреса
#ifdef __CONFIG_H__
	if(data[0] != g_sConfig.u8NetworkAddress)
#else
	if( (data[0] != 0) && (data[0] != NETWORK_ADDRESS) )
#endif
	{
		if(data[0] == 251) //additional header w/ chipID
		{
			if(len < (4+12)) return;

			uint8_t i;
			uint8_t chipid[12];
			for(i=0; i<3; i++)  //ТОDO перенести сhipid в g_chipid т формировать при запуске
			{
				memrcpy_u32(&chipid[i*4], gc_Unique_ID[i]);
			}

			for(i=0; i<12; i++)
			{
				if(chipid[i] != data[1+i]) return;
			}

			head = 1+12;  //Первый байт после адреса
		}
		else  //транзит на порт 1 (USART1)
//		if(data[0] != 0)  //* Не работаем с нулевыми адресами
		{
			UART4_send(data,len);
			return;
		}
	}

	g_fullNA = (data[0] == 251); //0xFB  Полный адрес

	uint16_t ccrc = ((u16)data[len-1]<<8) | data[len-2];
	uint16_t gcrc = crc16Modbus(data, len-2);

#if (FLAGS & FLAGS_MODBUS_DEBUG) == 0
	if(gcrc == ccrc)
#endif
	{
		tsCallback Cb;
		Cb.cb = FrameProcess;
		Cb.param = len-(2+head);
		TmpData = data+head;
		Callback_Save(&Cb);
		//FrameProcess(data+head, len-(2+head))
	}
	else
	{
		FrameError(0xff, 0xff);//DEBUG
	}
}
PUBLIC void FrameModbus_Send(uint8_t fun, uint8_t* data, uint8_t len)
{
	printf("--u8Mode=%d Modbus_Send--u8NetworkAddress == %d\n\r",g_sConfig.u8Mode, g_sConfig.u8NetworkAddress);

	if((len > 0) && (data == NULL)) return;

	uint8_t fdata[256];
	memset(fdata, 0, 256);

	uint16_t offset;
	uint16_t flen = 1+1+len+2;    //1(адрес)+1(номер функции)+ len (сами данные) +2 (crc)

	if(g_fullNA) {
		uint8_t i;
		fdata[0] = 251; //0xFB
		for(i = 0; i < 3; i++)
		{
			memrcpy_u32(&fdata[1 + i * 4], gc_Unique_ID[i]);
		}

		offset = 12;
		flen += 12;
	} else {
#ifdef __CONFIG_H__
		fdata[0]=g_sConfig.u8NetworkAddress;
#else
		fdata[0]=NETWORK_ADDRESS;
#endif
		offset = 0;
	}

	fdata[offset + 1] = fun;

	if(data != NULL)
	{
		memcpy(fdata + offset + 2, data, len);
	}

	uint16_t gcrc = crc16Modbus(fdata, flen-2);
	fdata[flen-2]=LOW(gcrc);
	fdata[flen-1]=HIGH(gcrc);
	//Usart3_send(fdata, flen);

	if(gFromUsart == UART5_NUMBER)
	{
		UART5_send(fdata, flen);
	}
	else{
		Usart3_send(fdata, flen);
	}
}



