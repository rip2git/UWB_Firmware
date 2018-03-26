#ifndef RANGING_H_
#define RANGING_H_


#include "Transceiver.h"
#include "MACFrame.h"


/*! ------------------------------------------------------------------------------------------------------------------
 * @def: Ranging_PAYLOAD_SIZE
 *
 * @brief: Payload size in the first ds-twr algorithm' message
 *
*/
#define Ranging_PAYLOAD_SIZE	6

/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: Ranging_RESULT
 *
 * @brief: Used for results (described below) returned from Ranging methods
 *
*/
typedef int Ranging_RESULT;

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: Ranging_ERROR
 *
 * @brief: Returns after failure operation
 *
*/
#define Ranging_ERROR 		(-1)

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: Ranging_SUCCESS
 *
 * @brief: Returns after successful operation
 *
*/
#define Ranging_SUCCESS 	(0)


/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Ranging_Initialization
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
extern void Ranging_Initialization(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Ranging_GetDistance
 *
 * @brief: 
 *
 * NOTE: 
 *
 * input parameters
 * @param header - header of the frame (ref to MACFrame.h)
 * 
 * output parameters
 * @param distance16 - measured distance from a initiator
 * @param header - changes 'flag' and 'sequence number' fields (ref to MACFrame.h)
 * 
 * return value is Ranging_RESULT described above
*/
extern Ranging_RESULT Ranging_GetDistance(MACHeader_Typedef *header, uint16_t *distance16);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Ranging_Initiate
 *
 * @brief: 
 *
 * NOTE: 
 *
 * input parameters
 * @param header - header of the frame (ref to MACFrame.h)
 * @param payload - payload (size of message is defined by Ranging_PAYLOAD_SIZE is discribed above) to another device
 * 
 * output parameters
 * @param header - changes 'flag' and 'sequence number' fields (ref to MACFrame.h)
 * 
 * return value is Ranging_RESULT described above
*/
extern Ranging_RESULT Ranging_Initiate(MACHeader_Typedef *header, const uint8_t *payload);


#endif