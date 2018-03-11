#ifndef USART_H_
#define USART_H_

#include "stm32f0xx.h"


#define USART_RX_EMPTY 					0
#define USART_OVERFLOW_RESET 			0


extern void USART_Initialization(void);
extern void USART_SendNewLine(void);
extern void USART_SendByte(const uint8_t c);
extern void USART_SendBuffer(const uint8_t *buffer, uint16_t length);
extern void USART_SendString(const char string[]);
extern uint8_t USART_GetByte(void);
extern inline uint16_t USART_GetRxStatus(void);
extern inline uint8_t USART_GetErrorStatus(void);
extern inline void USART_ResetErrorStatus(void);


#endif