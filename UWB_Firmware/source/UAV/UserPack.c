#include "UserPack.h"


uint8_t UserPack_ToBytes(const UserPack *pack, uint8_t *buffer)
{
	uint8_t i = 0;
	buffer[i++] = pack->FCmd._raw;
	buffer[i++] = pack->SCmd._raw;
	buffer[i++] = pack->TotalSize;
	for (uint8_t j = 0; j < pack->TotalSize; ++j)
		buffer[i++] = pack->Data[j];
	return i;
}



void UserPack_ToStruct(UserPack *pack, const uint8_t *buffer)
{
	uint8_t i = 0;
	pack->FCmd._raw = buffer[i++];
	pack->SCmd._raw = buffer[i++];
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
	pack->FCmd._raw = 0;
	pack->SCmd._raw = 0;
	pack->TotalSize = 0;
}


