#ifndef MACFRAME_H
#define MACFRAME_H


#include <stdint.h>


// MAC frame buffer offsets
#define MACFrame_CONTROL_OFFSET					0
#define MACFrame_SEQ_NUM_OFFSET					2
#define MACFrame_PAN_ID_OFFSET					3
#define MACFrame_DESTINATION_ADDRESS_OFFSET		5
#define MACFrame_SOURCE_ADDRESS_OFFSET			7
#define MACFrame_FLAGS_OFFSET 					9		// not standart field,
#define MACFrame_PAYLOAD_OFFSET 				10		// actually, payload starts from 9 offset (in this case, here is Flags field)

#define MACFrame_CONTROL_SIZE					2
#define MACFrame_SEQ_NUM_SIZE					1
#define MACFrame_PAN_ID_SIZE					2
#define MACFrame_DESTINATION_ADDRESS_SIZE		2
#define MACFrame_SOURCE_ADDRESS_SIZE			2

#define MACFrame_HEADER_SIZE					10		// with Flags field
#define MACFrame_FLAGS_SIZE 					1
#define MACFrame_FCS_SIZE						2


// Flags 
#define MACFrame_Flags_RNG			0x01
#define MACFrame_Flags_DATA			0x02
#define MACFrame_Flags_TOKEN		0x04
#define MACFrame_Flags_ACK			0x80



typedef struct {
	uint16_t 	FrameControl;	// according with IEEE Std 802.15.4-2011
	uint8_t 	SequenceNumber;	// according with IEEE Std 802.15.4-2011
	uint16_t 	PAN_ID;			// according with IEEE Std 802.15.4-2011
	uint16_t	DestinationID;	// according with IEEE Std 802.15.4-2011
	uint16_t	SourceID;		// according with IEEE Std 802.15.4-2011
	uint8_t		Flags;			// not standart field
} MACHeader_Typedef;


#endif