#ifndef USART_HANDLER_H
#define USART_HANDLER_H


#include "UserPack.h"


typedef int USARTHandler_RESULT;



#define USARTHandler_ERROR		(-1)
#define USARTHandler_SUCCESS	0



extern uint8_t USARTHandler_isAvailableToReceive(void);
extern void USARTHandler_Initialization(void);
extern USARTHandler_RESULT USARTHandler_Send(const UserPack *pack);
extern USARTHandler_RESULT USARTHandler_Receive(UserPack *pack);


#endif