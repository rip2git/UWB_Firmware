#ifndef USART_HANDLER_H
#define USART_HANDLER_H


#include "UserPack.h"


/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: USARTHandler_RESULT
 *
 * @brief: Used for results (described below) returned from USARTHandler methods
 *
*/
typedef int USARTHandler_RESULT;

/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: USARTHandler_BOOL
 *
 * @brief: Logical result of operations
 *
*/
typedef uint8_t USARTHandler_BOOL;

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: USARTHandler_ERROR
 *
 * @brief: Returns after failure operation
 *
*/
#define USARTHandler_ERROR		(-1)

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: USARTHandler_SUCCESS
 *
 * @brief: Returns after successful operation
 *
*/
#define USARTHandler_SUCCESS	0

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: USARTHandler_FALSE
 *
 * @brief: Returns if statement is false
 *
*/
#define USARTHandler_FALSE		(0)

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: USARTHandler_TRUE
 *
 * @brief: Returns if statement is true
 *
*/
#define USARTHandler_TRUE		(!USARTHandler_FALSE)


/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: USARTHandler_isAvailableToReceive
 *
 * @brief: Allows to check there is data from USART, returns USARTHandler_TRUE if it is
 *
 * NOTE: 
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
 * NOTE: 
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