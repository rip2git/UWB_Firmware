#ifndef MACFRAME_H
#define MACFRAME_H


#include <stdint.h>

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: MACFrame_OFFSETS
 *
 * @brief: MAC frame buffer offsets
 *
*/
#define MACFrame_CONTROL_OFFSET					0
#define MACFrame_SEQ_NUM_OFFSET					2
#define MACFrame_PAN_ID_OFFSET					3
#define MACFrame_DESTINATION_ADDRESS_OFFSET		5
#define MACFrame_SOURCE_ADDRESS_OFFSET			7
#define MACFrame_FLAGS_OFFSET 					9		// not standart field,
#define MACFrame_PAYLOAD_OFFSET 				10		// actually, payload starts from 9 offset 	
														// (in this case, here is Flags field)

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: MACFrame_SIZES
 *
 * @brief: Sizes of some fields of MAC frame
 *
*/
#define MACFrame_CONTROL_SIZE					2
#define MACFrame_SEQ_NUM_SIZE					1
#define MACFrame_PAN_ID_SIZE					2
#define MACFrame_DESTINATION_ADDRESS_SIZE		2
#define MACFrame_SOURCE_ADDRESS_SIZE			2
#define MACFrame_FLAGS_SIZE 					1
#define MACFrame_HEADER_SIZE					10
#define MACFrame_FCS_SIZE						2
#define MACFrame_FRAME_MAX_SIZE					127
#define MACFrame_PAYLOAD_MAX_SIZE				(MACFrame_FRAME_MAX_SIZE - MACFrame_HEADER_SIZE - MACFrame_FCS_SIZE)

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: Flags 
 *
 * @brief: Defines of bits of the 'Flags' field of MAC header
 *
*/
typedef enum {
	MACFrame_Flags_NOP = 0x00,
	MACFrame_Flags_RNG = 0x01,
	MACFrame_Flags_DATA = 0x02,
	MACFrame_Flags_TOKEN = 0x04,
	MACFrame_Flags_RET = 0x08,
	MACFrame_Flags_RST = 0x20,
	MACFrame_Flags_SYN = 0x40,
	MACFrame_Flags_ACK = 0x80
} MACFrame_Flags;

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: MACFrame_BROADCAST_ID
 *
 * @brief:
 *
*/
#define MACFrame_BROADCAST_ID		0xFFFF

/*! ------------------------------------------------------------------------------------------------------------------
 * Structure typedef: Transceiver_RxConfig
 *
 * @brief: header of MAC frame is according with IEEE Std 802.15.4-2011
 *
 * NOTE: first byte of payload (10th byte of message) used as header field named 'Flags' (not standard)
 *
*/
typedef struct {
	uint16_t 	FrameControl;
	uint16_t 	PAN_ID;
	uint16_t	DestinationID;
	uint16_t	SourceID;
	uint8_t 	SequenceNumber;
	MACFrame_Flags	Flags;
} MACHeader_Typedef;

#endif
