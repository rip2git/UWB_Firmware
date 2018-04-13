#ifndef USERPACK_H
#define USERPACK_H


#include <stdint.h>


/*! ------------------------------------------------------------------------------------------------------------------
 * @def: UserPack_BROADCAST_ID
 *
 * @brief: Broadcast ID in the PAN
 *
*/
#define UserPack_BROADCAST_ID 			0xFF

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: UserPack_OFFSETS
 *
 * @brief: offsets in the buffer contained user pack
 *
*/
#define UserPack_TOTAL_SIZE_OFFSET		2
#define UserPack_DATA_OFFSET			3

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: UserPack_SIZES
 *
 * @brief: sizes of entities in the buffer contained user pack
 *
*/
#define UserPack_SERVICE_INFO_SIZE 		3		// cmd + id + ttlsize
#define UserPack_FCS_SIZE		 		2		// crc16
#define UserPack_MAX_DATA_SIZE 			127		// according with IEEE Std 802.15.4-2011
#define UserPack_MAX_PACK_BYTE			(UserPack_MAX_DATA_SIZE + UserPack_SERVICE_INFO_SIZE + UserPack_FCS_SIZE)

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: UserPack_Commands
 *
 * @brief:
 *
*/
#define UserPack_Cmd_Status 			0
#define UserPack_Cmd_SetConfig			1
#define UserPack_Cmd_Distance			2
#define UserPack_Cmd_Data				3

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: UserPack_STATUSES
 *
 * @brief:
 *
*/
#define UserPack_STATUS_Reserved		0
#define UserPack_STATUS_SetConfig		1
#define UserPack_STATUS_Distance		2
#define UserPack_STATUS_Data			3

/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: UserPack struct
 *
 * @brief: 
 *
*/
typedef struct {
	uint8_t Command;
	uint8_t DestinationID;
	uint8_t TotalSize;
	uint8_t Data[UserPack_MAX_DATA_SIZE];
} UserPack;


/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: UserPack_ToBytes
 *
 * @brief: Converts structed pack to bytes. The data is put into the buffer
 *
 * NOTE: 
 *
 * input parameters
 * @param pack - structured data
 *
 * output parameters
 * @param buffer - unstructured data in direct order
 *
 * return value is size of buffers payload
*/
extern uint8_t UserPack_ToBytes(const UserPack *pack, uint8_t *buffer);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: UserPack_ToStruct
 *
 * @brief: Converts bytes to structed pack. The data is put into the pack
 *
 * NOTE: 
 *
 * input parameters
 * @param buffer - unstructured data in direct order
 *
 * output parameters
 * @param pack - structured data
 *
 * no return value
*/
extern void UserPack_ToStruct(UserPack *pack, const uint8_t *buffer);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: UserPack_SetData
 *
 * @brief: Put payload into structured pack
 *
 * NOTE: TotalSize must be defined before
 *
 * input parameters
 * @param buffer - payload (Data)
 *
 * output parameters
 * @param pack - structured data with payload
 *
 * no return value
*/
extern void UserPack_SetData(UserPack *pack, const uint8_t *buffer);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: UserPack_Reset
 *
 * @brief: Clears the content of pack
 *
 * NOTE: 
 *
 * input parameters
 * @param pack - pack that should be reseted
 *
 * output parameters
 * @param pack - Cleared pack
 *
 * no return value
*/
extern void UserPack_Reset(UserPack *pack);


#endif