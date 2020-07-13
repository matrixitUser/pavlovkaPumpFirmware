#ifndef __ADC1_H__
#define __ADC1_H__

#include "main.h"

#if !defined(ADC1_IRQ)
#error Adc1 IRQ is UNDEFINED!
#endif

PUBLIC void Adc1_Init(void);
PUBLIC void Adc1_Start(void);
PUBLIC void Adc1_sendValues(void);
PUBLIC uint16_t Adc1_getValue(uint8_t n);
PUBLIC uint16_t Adc1_getRaw(uint8_t n);

#endif /* __ADC1_H__ */
