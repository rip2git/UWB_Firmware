#ifndef ROUTING_H_
#define ROUTING_H_


#include "TokenExt.h"
#include "Transceiver.h"
#include "MACFrame.h"


/*! ------------------------------------------------------------------------------------------------------------------
 * @def: Routing_TABLE_SIZE
 *
 * @brief: Size of routing table (maximum number of devices in the network)
 *
 * NOTE: Every transferred token will include Routing_TABLE_SIZE bytes in its payload
 *
*/
#define Routing_TABLE_SIZE		10

/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: Routing_RESULT
 *
 * @brief: Used for results (described below) returned from Routing methods
 *
*/
typedef enum {
	Routing_INTERRUPT = -2,
	Routing_ERROR = -1,
	Routing_SUCCESS = 0
} Routing_RESULT;

/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: Routing_RESULT
 *
 * @brief: Used for results (described below) returned from Routing methods
 *
 * @param deviceID - todo
 * @param ACKReceivingTimeOut - timeout between for acknowledge after any request
 *
*/
typedef struct {
	uint8_t deviceID;
	uint8_t ACKReceivingTimeOut;
} Routing_InitializationStruct;


/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Routing_SendData
 *
 * @brief:
 *
 * NOTE:
 *
 * input parameters
 * @param header - header of the frame (ref to MACFrame.h)
 * @param payload - user data
 * @param payload_size - size of user data
 *
 * output parameters
 * @param header - changes DestinationID, Flags and SequenceNumber of header
 *
 * return value is Routing_RESULT
 * 	- Routing_FAIL if failed to send data
 * 	- Routing_SUCCESS if data was sent
*/
extern Routing_RESULT Routing_SendData(MACHeader_Typedef *header, const uint8_t *payload, uint8_t payload_size);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Routing_RecvDataTransferRequest
 *
 * @brief: todo
 *
 * NOTE: todo
 *
 * input parameters
 * @param header - header of the frame (ref to MACFrame.h)
 * @param rx_buffer - rx_buffer of Transceiver_RxConfig (e.g. after Transceiver_GetAvailableData) contained SYN
 * request
 * @param data_buffer_size - upper size of data_buffer
 *
 * output parameters
 * @param header - changes DestinationID, Flags and SequenceNumber of header
 * @param data_buffer - buffer that will contain the received data (frame + payload + fcs)
 * @param data_buffer_size - returned size of data_buffer. Equals to zero if there isn't available data. Can return
 * value less or equal then original data_buffer_size, but not greater.
 *
 * return value is Routing_RESULT:
 * 	- Routing_FAIL if failed to retransmit or receive the data
 * 	- Routing_SUCCESS if data was retransmitted or received
*/
extern Routing_RESULT Routing_RecvDataTransferRequest(
	MACHeader_Typedef *header,
	const uint8_t *rx_buffer,
	uint8_t *data_buffer,
	uint8_t *data_buffer_size
);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Routing_SendTokenWithTable
 *
 * @brief: Transfer TokenExt to next device of the PAN with routing table of this device
 *
 * NOTE:
 *
 * input parameters
 * @param header - header of the frame (ref to MACFrame.h)
 *
 * output parameters
 * @param header - changes DestinationID, Flags and SequenceNumber of header
 *
 * no return value
*/
extern void Routing_SendTokenWithTable(MACHeader_Typedef *header);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Routing_RecvTokenWithTable
 *
 * @brief: Receipt the TokenExt in devices order. Rejects the TokenExt if someone grabbed the TokenExt first. Also
 * collects routing table from transmitting device and changes own table according with received.
 *
 * NOTE:
 *
 * input parameters
 * @param header - header of the frame (ref to MACFrame.h)
 * @param rx_buffer - rx_buffer of Transceiver_RxConfig (e.g. after Transceiver_GetAvailableData) contained
 * extended token (with Routing table)
 *
 * output parameters
 * @param header - changes DestinationID, Flags and SequenceNumber of header
 *
 * no return value
*/
extern void Routing_RecvTokenWithTable(MACHeader_Typedef *header, const uint8_t *rx_buffer);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Routing_GetReturnedToken
 *
 * @brief: Wrapper for TokenExt_GetReturnedToken
 *
 * NOTE: Allows to safe old table's data
 *
 * input parameters
 * @param rx_buffer - rx_buffer of Transceiver_RxConfig (e.g. after Transceiver_GetAvailableData)
 *
 * return value is TokenExt_RESULT described above
*/
extern void Routing_GetReturnedToken(const uint8_t *rx_buffer);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Routing_GenerateToken
 *
 * @brief: Tries to generate the token
 *
 * NOTE:
 *
 * input parameters
 * @param header - header of the frame (ref to MACFrame.h)
 *
 * output parameters
 * @param header - changes DestinationID, Flags and SequenceNumber of header
 *
 * no return value
*/
extern void Routing_GenerateToken(MACHeader_Typedef *header);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Routing_Initialization
 *
 * @brief: Initializes internal entities required for working
 *
 * NOTE:
 *
 * input parameters
 * @param initializationStruct -
 *
 * output parameters
 *
 * no return value
*/
extern void Routing_Initialization(const Routing_InitializationStruct *initializationStruct);


#endif
