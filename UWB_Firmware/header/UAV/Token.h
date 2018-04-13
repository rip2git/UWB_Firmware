#ifndef TOKEN_
#define TOKEN_

#include "MACFrame.h"
#include <stdint.h>


/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: Token_RESULT
 *
 * @brief: Used for results (described below) returned from Token methods
 *
*/
typedef int Token_RESULT;

/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: Token_BOOL
 *
 * @brief: Logical result of operations
 *
*/
typedef uint8_t Token_BOOL;

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: Token_FAIL
 *
 * @brief: Returns after failure operation
 *
*/
#define Token_FAIL			(-1)

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: Token_SUCCESS
 *
 * @brief: Returns after successful operation
 *
*/
#define Token_SUCCESS		0

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: Token_FALSE
 *
 * @brief: Returns if statement is false
 *
*/
#define Token_FALSE		(0)

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: Token_TRUE
 *
 * @brief: Returns if statement is true
 *
*/
#define Token_TRUE		(!Token_FALSE)


/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Token_Transfer
 *
 * @brief: Transfer token to next device of the PAN
 *
 * NOTE: 
 *
 * input parameters
 * @param header - header of the frame (ref to MACFrame.h)
 *
 * output parameters
 * @param header - changes 'flag' and 'sequence number' fields (ref to MACFrame.h)
 *
 * return value is Token_RESULT described above
*/
extern Token_RESULT Token_Transfer(MACHeader_Typedef *header);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Token_Receipt
 *
 * @brief: Receipt the token in devices order. Rejects the token if someone grabbed the token first
 *
 * NOTE: 
 *
 * input parameters
 * @param header - header of the frame (ref to MACFrame.h)
 * @param tokenOwnerID - ID of device that sent the token
 *
 * output parameters
 * @param header - changes 'flag' and 'sequence number' fields (ref to MACFrame.h)
 *
 * return value is Token_RESULT described above
*/
extern Token_RESULT Token_Receipt(MACHeader_Typedef *header, uint8_t tokenOwnerID);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Token_Generate
 *
 * @brief: 
 *
 * NOTE: 
 *
 * input parameters
 * @param header - header of the frame (ref to MACFrame.h)
 *
 * output parameters
 * @param header - changes 'flag' and 'sequence number' fields (ref to MACFrame.h)
 *
 * return value is Token_RESULT described above
*/
extern Token_RESULT Token_Generate(MACHeader_Typedef *header);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Token_ImmediateReceipt
 *
 * @brief: Receipt the token without delay
 *
 * NOTE: 
 *
 * input parameters
 * @param header - header of the frame (ref to MACFrame.h)
 *
 * output parameters
 * @param header - changes 'flag' and 'sequence number' fields (ref to MACFrame.h)
 *
 * return value is Token_RESULT described above
*/
extern Token_RESULT Token_ImmediateReceipt(MACHeader_Typedef *header);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Token_isCaptured
 *
 * @brief: Allows to check state of token, returns Token_TRUE if token is captured
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * return value is Token_RESULT described above
*/
extern Token_BOOL Token_isCaptured();

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Token_SetMaxID
 *
 * @brief: Defines internal entities 
 *
 * NOTE: 
 *
 * input parameters
 * @param ID - higher ID in the PAN
 * @param timeSlotDurationMs - duration of token timeslot in ms (the pause between messages of two devices, 
 * when these devices try to catch the token)
 *
 * output parameters
 *
 * no return value
*/
extern void Token_Initialization(uint8_t ID, uint8_t timeSlotDurationMs);

#endif