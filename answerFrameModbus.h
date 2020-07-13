//masterFrameModbus.h
#include "main.h"
//#include "stdafx.h"

	//Павловка Soft Starter добавлен 06.07.2019 Ильнур

	#define NETWORK_ADDRESS_UPP1 							47//, crc=?
 	#define NETWORK_ADDRESS_WATER_LEVEL_SENSOR				16//, crc=?

	//Function Soft Starter - Функции устройства плавного пуска
	#define MODBUS_USER_FUNCTION_GET_STATUS_SOFTSTARTER		1

	#define MODBUS_USER_FUNCTION_SET_SOFTSTARTER			5

	//Биты состояния УПП, 1 функция Modbus
	#define MODBUS_REG_STOP_STATUS_SOFTSTARTER				1 //(0x02) //Состояние остановки УПП
	#define MODBUS_REG_START_STATUS_SOFTSTARTER				2 //(0x04) //Состояние запущенности УПП
	#define MODBUS_REG_AUTOMODE_STATUS_SOFTSTARTER			5 //Состояние Автоматического режима УПП
	#define MODBUS_REG_FAULT_STATUS_SOFTSTARTER				6 //Если есть какая-нибудь ошибка, то флаг выставляется в 1
	#define MODBUS_REG_DI_START_STATUS						11 //Hardwire DI Start input signal state
	#define MODBUS_REG_DI_STOP_STATUS						12 //Hardwire DI Stop input signal state
	#define MODBUS_REG_DI_RESET_STATUS						13 //Hardwire DI Reset input signal state
	#define	MODBUS_REG_TOR_STATUS							14 //
	#define	MODBUS_REG_MOTOR_START_STATUS					15 //CHeck stability of motor

	//Биты команды на УПП, 5 функция Modbus
	#define NO_COMMAND										0
	#define STOP_SOFTSTARTER								1 //Остановить двигатель
	#define START_SOFTSTARTER								2 //Запустить двигатель
	#define SET_AUTOMODE_SOFTSTARTER						5 //Установка автоматического режима для уудаленного управления УПП
	#define RESET_FAULT_SOFTSTARTER							6 //Сброс Ошибки
	#define RESET_FAULT_SOFTSTARTER							6 //Сброс Ошибки

	//Состояние пуска двигателя
	#define ENGINE_STOP_STATUS								0 // УПП остановлено
	#define ENGINE_START_STATUS								1 // УПП запущен
	#define ENGINE_STOPING_STATUS							2 // Состояние в процессе останвливания УПП
	#define ENGINE_STARTING_STATUS							3 // Состояние в процессе запуска УПП

	//Датчик уровня воды
	#define MODBUS_USER_FUNCTION_GET_STATUS_WATER_SENSOR	1
	#define MODBUS_REG_MIN_WATER_STATUS						1//Датчик нижнего предела воды
	#define MODBUS_REG_MIN_MIDDLE_WATER_STATUS				1//Датчик нижнего-среднего предела воды
	#define MODBUS_REG_MAX_MIDDLE_WATER_STATUS				2//Датчик верхнего-среднего предела воды
	#define MODBUS_REG_MAX_WATER_STATUS						2//Датчик верхнего предела воды

	PUBLIC void MasterFrameModbus_Init(void);
	uint8_t MasterModbusSend(uint8_t NA, uint8_t fun, uint8_t* data, uint8_t len, int8_t lengthWait, uint8_t tryes, uint8_t timeout);

	void answerFrameModbusDataProcess(uint8_t *data, uint16_t len);
	void sendTerminal(uint8_t NA, uint8_t* frame, uint16_t len, uint8_t sendOrReceive);
	void printff(char* frame, uint16_t len);
	//PRIVATE void MasterFrameProcess(uint16_t NAcrc, uint8_t* frame, uint16_t len);
	//PRIVATE void FrameProcessUPP(uint8_t* frame, uint16_t len);
	//PRIVATE void FrameProcessWLS(uint8_t* frame, uint16_t len);

