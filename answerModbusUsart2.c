#include "answerModbusUsart2.h"

#define TIME_CORRECTION_MAX_SEC		120
#define MAX_REG_BYTES				180

void answerModbusUsart2(uint8_t NA, uint8_t* frame, uint16_t len)
{
	uint8_t i;
	uint8_t fun = frame[0];
	uint8_t lengthWait = len-2-2; //?

	//Ответ от устройства
	if( (NA == req.NA) && (fun == req.fun) && ((req.lengthWait == -1 )||(req.lengthWait == lengthWait) )) //это ответ на нашу команду
	{
		g_isAnswer = 1;
		for(i=0;i<len;i++) {g_frame[i] = frame[i]; g_frameLen = len;}
	}
	else if(NA == req.NA)//*if((fun) == (req.fun+128))
	{
		if (fun == 0x07)//Функция выставления уровня логирования   ок двигатель останавливался?да
		{
			uint8_t data[1];

			gPrintf = frame[1];

			data[0] = gPrintf;
			MasterModbusSendUsart2(NA, fun, data, sizeof(data), lengthWait, 3, 1);
		}
		//TODO Отправка наверх (на сервер)
		//printf("NOT RIGHT COMMAND|\n\r");
		//Можно сюда добавить gPrintf!  думаю. да и если заренее известная функция
		//выходи звони
	}
	req.respSts=0;

}

//*Начало Изменено в связи с добавлением 2-го порта 15.11.19
void answerModbusUsart2DataProcess(uint8_t *data, uint16_t len)
{
	uint8_t fullNA = 0;
	uint8_t chipid[12];
	for(int i=0; i<12; i++)
	{
		chipid[i] = 0;
	}
	if(len < 1) return; //*01.07.19

	uint8_t head = 1;   //Первый байт после адреса
	//Получение адреса
	if(data[0] == 251) //0xFB additional header w/ chipID
	{
		fullNA=1;
		if(len < (4+12)) return;//?
		int i;
		for(i=0; i<12; i++)
		{
			if(chipid[i] != data[1+i]) return;
		}

		head = 1+12;  //Первый байт после адреса ?
	}
	else
	{
		chipid[0] = data[0];
		fullNA=0;
	}
	uint16_t ccrc = ((u16)data[len-1]<<8) | data[len-2]; //?
	uint16_t gcrc = crc16Modbus(data, len-2);//?

	if(gcrc == ccrc)
	{
		//uint16_t NAcrc = crc16Modbus(chipid, 12);//?crcNA
		answerModbusUsart2(data[0], data+head, len-(2+head));
	}
	_printf(2, "i2:");for(int jj=0;jj<len;jj++)	_printf(2, "%2x ",data[jj]);_printf(2, "\n\r");
}
