#ifndef USART_HANDLER_H
#define USART_HANDLER_H


#include "UserPack.h"


/*! ------------------------------------------------------------------------------------------------------------------
 * @def: USART_DMA_VERSION
 *
 * @brief: Switch working mode of USART: 
 * - defined: using USART through DMA
 * - undef: using IRQ
 *
*/
#define USART_DMA_VERSION

/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: USARTHandler_RESULT
 *
 * @brief: Used for results (described below) returned from USARTHandler methods
 *
*/
typedef enum {
	USARTHandler_ERROR = -1,
	USARTHandler_SUCCESS = 0
} USARTHandler_RESULT;

/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: USARTHandler_BOOL
 *
 * @brief: Logical result of operations
 *
*/
typedef enum {
	USARTHandler_FALSE = 0,
	USARTHandler_TRUE = (!USARTHandler_FALSE)
} USARTHandler_BOOL;


/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USARTHandler_isAvailableToReceive
 *
 * @brief: Allows to check there is data from USART, returns USARTHandler_TRUE if it is
 *
 * NOTE: if DMA is using (@def USART_DMA_VERSION) then it is need to receive the data (by @fn USARTHandler_Receive) 
 * before calling this method again, otherwise the data may be lost
 *
 * input parameters
 *
 * output parameters
 *
 * return value is USARTHandler_BOOL described above
*/
extern USARTHandler_BOOL USARTHandler_isAvailableToReceive(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USARTHandler_Initialization
 *
 * @brief: Initializes internal entities required for working
 *
 * NOTE: starts read
 *
 * input parameters
 *
 * output parameters
 *
 * no return value
*/
extern void USARTHandler_Initialization(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USARTHandler_Send
 *
 * @brief: Send the pack through USART
 *
 * NOTE: 
 *
 * input parameters
 * @param pack - user pack (ref UserPack.h)
 *
 * output parameters
 *
 * return value is USARTHandler_RESULT described above
*/
extern USARTHandler_RESULT USARTHandler_Send(const UserPack *pack);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USARTHandler_Receive
 *
 * @brief: Receives the pack from USART
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 * @param pack - user pack (ref UserPack.h)
 *
 * return value is USARTHandler_RESULT described above
*/
extern USARTHandler_RESULT USARTHandler_Receive(UserPack *pack);


#endif
