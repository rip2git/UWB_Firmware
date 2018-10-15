#ifndef USERPACK_H
#define USERPACK_H


#include "MACFrame.h"


/*! ------------------------------------------------------------------------------------------------------------------
 * @def: UserPack_BROADCAST_ID
 *
 * @brief: Broadcast ID in the PAN
 *
*/
#define UserPack_BROADCAST_ID 			0xFFFF

#define UserPack_PAYLOAD_SIZE			12

/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: UserPack struct
 *
 * @brief: 
 *
*/
#pragma pack(push, 1)
typedef struct {
	uint16_t id;
	uint8_t payload[UserPack_PAYLOAD_SIZE];
	uint16_t distance;
} UserPack;
#pragma pack(pop)


#define UserPack_PAYLOAD_OFFSET			2
#define UserPack_DISTANCE_OFFSET		14
#define UserPack_PACK_SIZE				sizeof(UserPack)


#endif
