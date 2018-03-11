#include "UserPack.h"
#include "string.h"


uint8_t UserPack_ToBytes(const UserPack *pack, uint8_t *buffer)
{
	uint8_t i = 0;
	buffer[i++] = pack->Command;
	buffer[i++] = pack->DestinationID;
	buffer[i++] = pack->TotalSize;
	for (uint8_t j = 0; j < pack->TotalSize; ++j)
		buffer[i++] = pack->Data[j];
	return i;
}



void UserPack_ToStruct(UserPack *pack, const uint8_t *buffer)
{
	uint8_t i = 0;
	pack->Command = buffer[i++];
	pack->DestinationID = buffer[i++];
	pack->TotalSize = buffer[i++];
	for (uint8_t j = 0; j < pack->TotalSize; ++j)
		pack->Data[j] = buffer[i++];
}



void UserPack_SetData(UserPack *pack, const uint8_t *buffer)
{
	for (uint8_t i = 0; i < pack->TotalSize; ++i)
		pack->Data[i] = buffer[i];

}



void UserPack_Reset(UserPack *pack)
{
	pack->Command = UserPack_Cmd_Status;
	pack->DestinationID = UserPack_STATUS_Reserved;
	pack->TotalSize = 0;
}


