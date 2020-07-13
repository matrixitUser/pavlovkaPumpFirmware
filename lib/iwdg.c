#include "iwdg.h"

void IWDG_Init(void)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(IWDG_Prescaler_256); //IWDG counter clock: 32KHz(LSI) / 256 = 125Hz
    IWDG_SetReload(0xFFF); //Set counter reload value to 4095 => 32.768sec

    IWDG_ReloadCounter();
    IWDG_Enable();
}
