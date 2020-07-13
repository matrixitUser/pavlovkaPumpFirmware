#define __STRUCT_H__

#define __V2_1__

//#define COUNTERS		1 //*16
#define TEXT_LEN_MAX	(16 - 1)

#define CONFIG_CHANNEL_FLAG_ERROR		0x0001

// внутри 32-битной записи Counter содержится: значение счётчика, 8 разрядов (99999.999); значение фиксированной запятой (-4...+3); бит "Канал включен"; бит "Некорректное значение"
/*#define ARCHIVERECORD_COUNTER_VALUE_MASK		(0x07FFFFFFUL)
#define ARCHIVERECORD_COUNTER_POINT_MASK		(0x38000000UL)
#define ARCHIVERECORD_COUNTER_POINT_DIV			(27)
#define ARCHIVERECORD_COUNTER_ISENABLE_MASK		(0x80000000UL)
#define ARCHIVERECORD_COUNTER_ISENABLE_DIV		(31)
#define ARCHIVERECORD_COUNTER_ISERROR_MASK		(0x40000000UL)
#define ARCHIVERECORD_COUNTER_ISERROR_DIV		(30)
#define ARCHIVERECORD_COUNTER_GET_VALUE(x)		((x) & ARCHIVERECORD_COUNTER_VALUE_MASK)
#define ARCHIVERECORD_COUNTER_GET_POINT(x)		(((x) & ARCHIVERECORD_COUNTER_POINT_MASK) >> ARCHIVERECORD_COUNTER_POINT_DIV)*/

#define ARCHIVERECORD_COUNTER_VALUEMASK			(0x00FFFFFFUL)
#define ARCHIVERECORD_COUNTER_ENABLEMASK		(0x80000000UL)
#define ARCHIVERECORD_COUNTER_ERRORMASK			(0x40000000UL)
#define ARCHIVERECORD_COUNTER_PIN1MASK			(0x20000000UL)
#define ARCHIVERECORD_COUNTER_PIN0MASK			(0x10000000UL)

#define MODE_SERVICE	0
#define MODE_WORK		1


#define MODE_MASTER 	2
#define MODE_SLAVE		1
#define MODE_UNKNOWN	0

#define SECOND_UPP_NOT_USED    	0
#define SECOND_UPP_IS_USED     	1

#define WLSMODE_SERVER_ONLY 	2
#define WLSMODE_MC_SERVER		1
#define WLSMODE_MC_ONLY			0


#define FIRST_UPP_LEFT			0
#define FIRST_UPP_RIGHT			1

typedef struct
{
	u32 u32BaudRate;
	u8 u8WordLen;
	u8 u8StopBits;
	u8 u8Parity;
	u8 reserved;
} tsUartConfig;  //8 байт




#ifdef __V2_1__

#ifdef PAVLOVKA
typedef struct
{
	u16 u16FlashVer;					//0x00  (0)
	u8 u8NetworkAddress;				//0x02
	u8 u8Mode;							//master or slave  master===1 / slave===0

	uint32_t u32ReleaseTs;

	uint8_t u8IsRtcError;//0x03
	uint8_t UPP_NA_RIGHT;
	uint8_t UPP_NA_LEFT;
	uint8_t WLS_NA;

	tsUartConfig sUart1;	//
	tsUartConfig sUart2;
	tsUartConfig sUart3;
	tsUartConfig sUart4;
	tsUartConfig sUart5;

	uint8_t PumpTimeIdleMax;
	uint8_t WlsServerMode;  			//0 - only mc, 1 - mc or server, 2 - only server
	uint8_t FirstUPP;//0x03				//0 - left, 1 - right
	uint8_t MinTimeAfterStart;//0x03

	uint8_t isUsedSecondUpp;			//0 - second upp not used, 1 - second has to use
	uint8_t u8Reserved1;
	uint8_t u8Reserved2;
	uint8_t u8Reserved3;

} tsConfig;

#endif

typedef struct
{
	uint32_t u32Ts;
	uint32_t u32AeId;
}
tsAbnormalEvent;


typedef struct
{
	uint32_t u32Ts;
	uint16_t u16eID;
}
tsEvent;


typedef struct
{
	uint32_t u32Ts;						//4
	uint32_t au32Counter[COUNTERS];		//+64=68
}
tsArchiveRecordPack;

typedef struct
{
	uint32_t u8Stat_u24Counter;
	int8_t i8Point;
	uint8_t u8Unit;
}
tsArchiveCounter;

typedef struct
{
	uint32_t u32Ts;
	tsArchiveCounter asCounter[COUNTERS];
}
tsArchiveRecordUnpack;

//Павловка

typedef struct //структура Датчика уровня воды
{
	uint8_t NA; //адрес датчика
	uint16_t oldStatus;
	uint16_t status;
	uint8_t minLvl; //флаг минимального уровня
	uint8_t maxLvl; //флаг максимального уровня
	uint8_t respSts; //флаг ответа
	uint8_t rqstTry; //количество попыток опроса ДУ
	uint32_t lastRespTime; //время последнего опроса устройства Owen
	float height;
} levelSensor;

typedef struct //структура УПП
{
	uint8_t NA; //адрес датчика
	uint16_t oldStatus;
	uint16_t status; //0-Автономный; 1-Дистанционный
	uint8_t engineSts; //состояния работы двигателя 0 - двигатель остановлен, 1 - двигател работает, 2 -
	uint8_t respSts; //флаг ответа
	uint8_t rqstTry; //количество попыток опроса
	uint32_t lastRespTime; //время последнего опроса УПП
	uint32_t lastStartTime; //время последнего старта УПП
	uint32_t lastStopTime; //время последнего стопа УПП
	uint16_t motorCurrent; //ток в % от Ie
	uint16_t thermalLoad;  //тепловая нагрузка в % от t срабатывания
	uint16_t currentPhase1; //ток фазы 1
	uint16_t currentPhase2; //ток фазы 2
	uint16_t currentPhase3; //ток фазы 3
	uint16_t currentPhaseMax; //максимальный ток по фазам
	uint16_t frequency; //частота тока
	uint16_t power;     //мощность 100 -> 1 Вт
	uint16_t voltage;   //напряжение в %
	uint16_t startCount; //количество стартов  1  -> 100 стартам
	uint16_t runtimeHours; //время работы    1-> часам
	uint16_t modbusError;
	uint16_t modbusToglle;
} softStarter;




typedef struct
{
	uint8_t NA;
	uint8_t fun;
	uint8_t respSts;
	uint8_t numCommand;
	int8_t lengthWait;
} rqstStruct;

#endif
