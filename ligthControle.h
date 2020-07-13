#include "stdafx.h"
#include "main.h"
#define __SETLIGTH__


void ligthControl_init(void);
void set_ligth(uint8_t u8Action);
void shedulle(uint8_t hdl, u32 time);
void photosensorCheckIn(uint8_t hdl, u32 time);
void astr_timer(u8 hdl, u32 time);
void alarm5sec(u8 hdl, u32 time);
void sendError();
void sendEvent(void);
uint8_t set_by_astr_time(u32 time,uint8_t Channel);
void set_ligthByChannel(uint8_t nChannel, uint8_t onOff);
void ligthControle(u8 hdl, u32 time);
