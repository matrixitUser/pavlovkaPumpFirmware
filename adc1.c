#include "main.h"

#ifdef __ADC1_H__

/////////////////////////////////////////////////////////////////_ x//
//// CONFIG //////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////
#define ADC_CHANNELS	2	// 2 канала в текущем варианте	только 2//
#define ADC_CALC 		0	// вычисление среднего			0 или 1	//
#define ADC_AUTORUN		0	// автозапуск (кушает ток)		0 или 1	//
// автозапуск разрешён только при использовании DMA или прерываний	//
#define ADC_USE_DMA		1	// использование DMA			0 или 1	//
#define ADC_USE_ISR		0	// использование прерываний		0 или 1	//
//////////////////////////////////////////////////////////////////////
#if ADC_CALC == 1
#define ADC_SEQUENCES	(8 + 2) //среднее от 8 значений, отбрасываем 2 экстремума
#define ADC_DIV_FACTOR	3		//деление на 8
#endif
//////////////////////////////////////////////////////////////////////
#if ADC_CALC == 1
PRIVATE __IO uint32_t _sumAdcCalc[ADC_CHANNELS];//[ADC_SEQUENCES];
PRIVATE __IO uint16_t _adcCalcMin[ADC_CHANNELS];//[ADC_SEQUENCES];
PRIVATE __IO uint16_t _adcCalcMax[ADC_CHANNELS];//[ADC_SEQUENCES];
PRIVATE uint8_t seqN;
#endif

PRIVATE __IO uint16_t _adcView[ADC_CHANNELS];
PRIVATE uint8_t adcN;


PUBLIC uint16_t Adc1_getValue(uint8_t n)
{
	return (n < ADC_CHANNELS)? _adcView[n] : 0;
}

PUBLIC void Adc1_Start(void)
{
#if (ADC_AUTORUN == 0)
#	if (ADC_USE_DMA == 1)
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
	while(!DMA_GetFlagStatus(DMA1_FLAG_TC1))
	{

	}

	//while(!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));
#	elif (ADC_USE_ISR == 1)
#	else
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);
	while(!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));
	_adcView[0] = ADC_GetConversionValue(ADC1);
	ADC_SoftwareStartConvCmd(ADC1,ENABLE);
	while(!ADC_GetFlagStatus(ADC1,ADC_FLAG_EOC));
	_adcView[1] = ADC_GetConversionValue(ADC1);
#	endif
#else
#	if ((ADC_USE_DMA == 0) && (ADC_USE_ISR == 0))
#		error "Please use autorun with DMA or ISR"
#	endif
#endif
}



PUBLIC uint16_t Adc1_getRaw(uint8_t channel)
{
	if(channel <= 18)
	{
		switch(channel)
		{
		case 16:
			return _adcView[0];

		case 17:
			return _adcView[1];
		}
	}
	return 0;
}

void Adc1_Init(void)
{
	uint8_t i;
	for(i=0; i<ADC_CHANNELS; i++)
	{
		_adcView[i] = 0;
	}
	adcN = 0;

#if (ADC_USE_DMA == 1)
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

	DMA_InitTypeDef DMA_InitStructure;
	DMA_InitStructure.DMA_BufferSize = 2;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)_adcView;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (uint32_t)&ADC1->DR;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_Priority = DMA_Priority_High;
	DMA_Init(DMA1_Channel1, &DMA_InitStructure);
	DMA_Cmd(DMA1_Channel1, ENABLE);
#endif

	RCC_ADCCLKConfig(RCC_PCLK2_Div6);

	/* ADC1 Periph clock enable */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);

	ADC_InitTypeDef ADC_InitStructure;
	ADC_StructInit(&ADC_InitStructure);
#if (ADC_AUTORUN == 1)
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;
#else
	ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
#endif
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None;
	ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;
#if (ADC_USE_DMA == 1)
	ADC_InitStructure.ADC_NbrOfChannel = 2;
	ADC_InitStructure.ADC_ScanConvMode = ENABLE;
#else
	ADC_InitStructure.ADC_NbrOfChannel = 2;
	ADC_InitStructure.ADC_ScanConvMode = DISABLE;
#endif
	ADC_Init(ADC1, &ADC_InitStructure);

	ADC_RegularChannelConfig(ADC1, ADC_Channel_16, 1, ADC_SampleTime_41Cycles5);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_17, 2, ADC_SampleTime_41Cycles5);
	ADC_TempSensorVrefintCmd(ENABLE);
	ADC_Cmd(ADC1, ENABLE);
#if (ADC_USE_DMA == 1)
	ADC_DMACmd(ADC1, ENABLE);
#endif

#if (ADC_USE_ISR == 1)
	ADC_ITConfig(ADC1, ADC_IT_EOC, ENABLE);
	NVIC_EnableIRQ(ADC1_IRQn);
	NVIC_SetPriority(ADC1_IRQn, NVIC_PRIORITY_ADC);
#endif

	//Before starting a calibration, the ADC must have been in power-on state (ADON bit = ‘1’) for
	//at least two ADC clock cycles.

	ADC_ResetCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));
	ADC_StartCalibration(ADC1);
	while(ADC_GetCalibrationStatus(ADC1));

#if (ADC_AUTORUN == 1)
	ADC_SoftwareStartConvCmd(ADC1, ENABLE);
#endif
}

#if (ADC_USE_ISR == 1)
void ADC1_IRQ(void)
{
    if(ADC_GetITStatus(ADC1, ADC_IT_EOC))
    {
    	ADC_ClearITPendingBit(ADC1, ADC_IT_EOC);
    	if(adcN < ADC_CHANNELS)
    	{
    		uint16_t adcValue = ADC_GetConversionValue(ADC1);
#if ADC_CALC == 1

    		_sumAdcCalc[adcN] += adcValue;
    		if(seqN == 0)
    		{
    			_adcCalcMin[adcN] = adcValue;
    			_adcCalcMax[adcN] = adcValue;
    		}
    		else
    		{
        		if(adcValue < _adcCalcMin[adcN])
    			{
        			_adcCalcMin[adcN] = adcValue;
        		}
        		if(adcValue > _adcCalcMax[adcN])
    			{
        			_adcCalcMax[adcN] = adcValue;
        		}
    		}
#else
    		_adcView[adcN] = adcValue;
#endif
/*
    		DEBUG_sendStr("adc");
    		DEBUG_sendDec(adcN, 1);
    		DEBUG_sendStr("=");
    		DEBUG_sendHex(adcValue, 4);
    		DEBUG_sendStr(",");*/

    		adcN++;
    		if(adcN == ADC_CHANNELS)
    		{
    			adcN = 0;
#if ADC_CALC == 1
        		DEBUG_sendStr("(");
        		DEBUG_sendDec(seqN, 1);
        		DEBUG_sendStr(")\r\n");

	    		seqN++;
    	    	if(seqN == ADC_SEQUENCES)
    	    	{
    	    		seqN = 0;
#endif
    				//ADC_StopOfConversion(ADC1);
    				//DEBUG_sendStr("[EOSEQ]");
    				//Sleep_Config(SLEEP_FLAG_ADC, RESET);
#if ADC_CALC == 1
    				uint8_t i;
    				for(i=0; i<ADC_CHANNELS; i++)
    				{
    					_adcView[i] = (_sumAdcCalc[i] - _adcCalcMin[i] - _adcCalcMax[i]) >> ADC_DIV_FACTOR;
    				}
    	    	}
#endif
    		}
    	}
    }

    //if(ADC_GetITStatus(ADC1, ADC_IT_EOSEQ))
    //{
	//	ADC_ClearITPendingBit(ADC1, ADC_IT_EOSEQ);
    //}
}
#endif

#endif
