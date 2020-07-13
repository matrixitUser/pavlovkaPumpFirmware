
#include "main.h"

	#define TALKING_BTWN_CONTROLLERS					0
	#define TALKING_ASKING_WLS							1
	#define TALKING_SWITCH_MASTER						2
	#define TALKING_UPP_RQST							3
	#define TALKING_UPP_START							4
	#define TALKING_UPP_STOP							5

	//NUM CMD pavlovka
	#define MODBUS_CMD_SEND								0
	#define MODBUS_CMD_ANSWER							1

	void AnswerControllers(uint8_t *data, uint16_t len);
	void ParseDataFromController(uint8_t* data);
	void TalkingBtwnCtrls_Send(uint8_t fun, uint8_t numCmd, uint8_t* data, uint8_t len);
	void ParseDataByFunc(uint8_t fun, uint8_t cmd, uint8_t* frame, uint16_t len);
	PUBLIC void CMDForTalkingBetweenControllers(uint8_t *ret);
	PUBLIC void AnswerForAskingWLS(u8 wlsOk);
	void TalkingCall(u8 hdl);
	void TalkingCallbackParse(u8 hdl, u32 param);
	PUBLIC void ArrFillDATANULL(uint8_t *ret);
