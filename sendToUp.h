#include "main.h"

void error();
void sendErrorTry();
void event();
void sendEventTry();
void sendMesagge(uint8_t errorOrevent, uint16_t message);
uint8_t fillMessage(uint8_t *ret,uint8_t len);
