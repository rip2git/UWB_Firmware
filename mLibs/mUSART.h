#ifndef USART_H_
#define USART_H_

#include "_DEBUG.h"
#include "stm32f0xx.h"


/*! ------------------------------------------------------------------------------------------------------------------
 * @def: USART_RX_EMPTY
 *
 * @brief: Returns from USART_GetRxCounter if receiving buffer is empty
 *
*/
#define USART_RX_EMPTY 					0

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: USART_NO_ERROR
 *
 * @brief: Returns from USART_GetErrorStatus if there were no errors
 *
*/
#define USART_NO_ERROR					0

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: USARTx_IRQHandler
 *
 * @brief: Internal IRQHandler name
 *
*/
#define USARTx_IRQHandler(x)		USART1_IRQHandler(x)

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USART_Initialization
 *
 * @brief: Initializes peripherals
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * no return value
*/
extern void USART_Initialization(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USART_SendNewLine
 *
 * @brief: Sends '\r\n' through USART
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * no return value
*/
extern void USART_SendNewLine(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USART_SendByte
 *
 * @brief: 
 *
 * NOTE: 
 *
 * input parameters
 * @param c - byte to be sent through USART
 *
 * output parameters
 *
 * no return value
*/
extern void USART_SendByte(const uint8_t c);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USART_SendBuffer
 *
 * @brief: 
 *
 * NOTE: 
 *
 * input parameters
 * @param buffer - array of the bytes to be sent through USART
 * @param length - 
 *
 * output parameters
 *
 * no return value
*/
extern void USART_SendBuffer(const uint8_t *buffer, uint16_t length);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USART_SendString
 *
 * @brief: Sends sequence of characters through USART
 *
 * NOTE: param string should be ended with null-terminator
 *
 * input parameters
 * @param string - sequence of characters that should be ended with null-terminator
 *
 * output parameters
 *
 * no return value
*/
extern void USART_SendString(const char *string);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USART_GetByte
 *
 * @brief: Get byte received from USART
 *
 * NOTE: This method can returns no valid value (= 0) if the receiving errors has occurred
 *
 * input parameters
 *
 * output parameters
 *
 * return value is a byte received from USART
*/
extern uint8_t USART_GetByte(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USART_GetRxCounter
 *
 * @brief: Get the number of bytes which is available to read
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * return value is the number of bytes which is available to read
*/
extern inline uint16_t USART_GetRxCounter(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USART_GetErrorStatus
 *
 * @brief: Returns the current error status
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * return value is the current error status
*/
extern inline uint8_t USART_GetErrorStatus(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USART_ResetErrorStatus
 *
 * @brief: Resets the error status
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * no return value
*/
extern inline void USART_ResetErrorStatus(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USART_ResetRxBuffer
 *
 * @brief: Resets receiving buffer (e.g. when the receiving errors has occurred)
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * no return value
*/
extern inline void USART_ResetRxBuffer(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USARTx_IRQHandler
 *
 * @brief: IRQ handler that be called when errors of receiving, rx or tx events would happen
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * no return value
*/
extern void USARTx_IRQHandler(void);


#endif