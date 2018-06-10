#ifndef TokenExt_H
#define TokenExt_H

#include "MACFrame.h"


/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: TokenExt_RESULT
 *
 * @brief: Used for results (described below) returned from TokenExt methods
 *
*/
typedef enum {
	TokenExt_FAIL = -1,
	TokenExt_SUCCESS = 0,
	TokenExt_NEWCYCLE
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
 * @def: TokenExt FIELDS SIZES
 *
 * @brief:
 *
*/
#define TokenExt_TOKEN_ID_SIZE				1
#define TokenExt_SERVICE_INFO				(MACFrame_HEADER_SIZE + TokenExt_TOKEN_ID_SIZE + 2)  // + 2 fcs
#define TokenExt_PAYLOAD_MAX_SIZE			(MACFrame_PAYLOAD_MAX_SIZE - TokenExt_TOKEN_ID_SIZE)

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: TokenExt FIELDS OFFSETS
 *
 * @brief:
 *
*/
#define TokenExt_TOKEN_ID_OFFSET		MACFrame_PAYLOAD_OFFSET
#define TokenExt_PAYLOAD_OFFSET			(TokenExt_TOKEN_ID_OFFSET + TokenExt_TOKEN_ID_SIZE)

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: TokenExt_Transfer
 *
 * @brief: Transfer TokenExt to next device of the PAN
 *
 * NOTE: 
 *
 * input parameters
 * @param header - header of the frame (ref to MACFrame.h)
 * @param payload - additional allowed user's bytes
 * @param payload_size - size of payload
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
 * @param rx_buffer - rx_buffer of Transceiver_RxConfig (e.g. after Transceiver_GetAvailableData)
 *
 * output parameters
 * @param header - changes 'flag' and 'sequence number' fields (ref to MACFrame.h)
 *
 * return value is TokenExt_RESULT described above
*/
extern TokenExt_RESULT TokenExt_Receipt(MACHeader_Typedef *header, const uint8_t *rx_buffer);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: TokenExt_GetReturnedToken
 *
 * @brief: Gets the TokenExt from descendent back or rejects it if some errors were occurred
 *
 * NOTE: 
 *
 * input parameters
 * @param rx_buffer - rx_buffer of Transceiver_RxConfig (e.g. after Transceiver_GetAvailableData)
 *
 * return value is TokenExt_RESULT described above
*/
extern TokenExt_RESULT TokenExt_GetReturnedToken(const uint8_t *rx_buffer);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: TokenExt_Generate
 *
 * @brief: Listen for other devices. If no one is tranceiving then catches the token.
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
extern TokenExt_RESULT TokenExt_Generate(MACHeader_Typedef *header);

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
 * @fn: TokenExt_Reset
 *
 * @brief:
 *
 * NOTE:
 *
 * input parameters
 *
 * output parameters
 *
 * no return value
*/
extern void TokenExt_Reset();

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: TokenExt_SetMaxID
 *
 * @brief: Defines internal entities 
 *
 * NOTE: 
 *
 * input parameters
 * @param maxID - higher ID in the PAN
 * @param selfID - this device ID
 * @param timeSlotDurationMs - duration of TokenExt timeslot in ms (the pause between messages of two devices,
 * when these devices try to catch the TokenExt)
 *
 * output parameters
 *
 * no return value
*/
extern void TokenExt_Initialization(uint8_t selfID, uint8_t maxID, uint8_t timeSlotDurationMs);

#endif
