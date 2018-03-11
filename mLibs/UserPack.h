#ifndef USERPACK_H
#define USERPACK_H



#include <stdint.h>



#define UserPack_BROADCAST_ID 			0xFF
#define UserPack_TOTAL_SIZE_OFFSET		2
#define UserPack_DATA_OFFSET			3
#define UserPack_SERVICE_INFO_SIZE 		5
#define UserPack_MAX_DATA_SIZE 			127 	// according with IEEE Std 802.15.4-2011
#define UserPack_MAX_PACK_BYTE			(UserPack_MAX_DATA_SIZE + UserPack_SERVICE_INFO_SIZE)

#define UserPack_Cmd_Status 		0
#define UserPack_Cmd_SetID			1
#define UserPack_Cmd_Distance		2
#define UserPack_Cmd_Data			3


#define UserPack_STATUS_Reserved		0
#define UserPack_STATUS_SetID			1
#define UserPack_STATUS_Distance		2
#define UserPack_STATUS_Data			3


typedef struct {
	uint8_t Command;
	uint8_t DestinationID;
	uint8_t TotalSize;
	uint8_t Data[UserPack_MAX_DATA_SIZE];
} UserPack;



extern uint8_t UserPack_ToBytes(const UserPack *pack, uint8_t *buffer);
extern void UserPack_ToStruct(UserPack *pack, const uint8_t *buffer);
extern void UserPack_SetData(UserPack *pack, const uint8_t *buffer);
extern void UserPack_Reset(UserPack *pack);


#endif