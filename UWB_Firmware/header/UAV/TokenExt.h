#ifndef TokenExt_
#define TokenExt_

#include "MACFrame.h"


/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: TokenExt_RESULT
 *
 * @brief: Used for results (described below) returned from TokenExt methods
 *
*/
typedef enum {
	TokenExt_FAIL = -1,
	TokenExt_SUCCESS = 0
} TokenExt_RESULT;

/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: TokenExt_BOOL
 *
 * @brief: Logical result of operations
 *
*/
typedef enum {
	TokenExt_FALSE = 0,
	TokenExt_TRUE = (!TokenExt_FALSE)
} TokenExt_BOOL;

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: TokenExt_MAX_PAYLOAD_SIZE
 *
 * @brief:
 *
*/
#define TokenExt_MAX_PAYLOAD_SIZE			10


/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: TokenExt_Transfer
 *
 * @brief: Transfer TokenExt to next device of the PAN
 *
 * NOTE: 
 *
 * input parameters
 * @param header - header of the frame (ref to MACFrame.h)
 * @param payload -
 * @param payload_size -
 *
 * output parameters
 * @param header - changes 'sequence number' field, adds own bits to 'flags' field (ref to MACFrame.h)
 *
 * return value is TokenExt_RESULT described above
*/
extern TokenExt_RESULT TokenExt_Transfer(MACHeader_Typedef *header, const uint8_t *payload, uint8_t payload_size);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: TokenExt_Receipt
 *
 * @brief: Receipt the TokenExt in devices order. Rejects the TokenExt if someone grabbed the TokenExt first
 *
 * NOTE: 
 *
 * input parameters
 * @param header - header of the frame (ref to MACFrame.h)
 * @param tokenOwnerID - ID of device that sent the TokenExt
 *
 * output parameters
 * @param header - changes 'flag' and 'sequence number' fields (ref to MACFrame.h)
 *
 * return value is TokenExt_RESULT described above
*/
extern TokenExt_RESULT TokenExt_Receipt(MACHeader_Typedef *header, uint8_t tokenOwnerID);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: TokenExt_ImmediateReceipt
 *
 * @brief: Receipt the TokenExt without delay
 *
 * NOTE: 
 *
 * input parameters
 * @param header - header of the frame (ref to MACFrame.h)
 *
 * output parameters
 * @param header - changes 'flag' and 'sequence number' fields (ref to MACFrame.h)
 *
 * return value is TokenExt_RESULT described above
*/
extern TokenExt_RESULT TokenExt_ImmediateReceipt(MACHeader_Typedef *header);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: TokenExt_isCaptured
 *
 * @brief: Allows to check state of TokenExt, returns TokenExt_TRUE if TokenExt is captured
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * return value is TokenExt_RESULT described above
*/
extern TokenExt_BOOL TokenExt_isCaptured();

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: TokenExt_SetMaxID
 *
 * @brief: Defines internal entities 
 *
 * NOTE: 
 *
 * input parameters
 * @param ID - higher ID in the PAN
 * @param timeSlotDurationMs - duration of TokenExt timeslot in ms (the pause between messages of two devices,
 * when these devices try to catch the TokenExt)
 *
 * output parameters
 *
 * no return value
*/
extern void TokenExt_Initialization(uint8_t ID, uint8_t timeSlotDurationMs);

#endif
