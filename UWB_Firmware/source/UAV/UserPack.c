#include "UserPack.h"


uint8_t UserPack_ToBytes(const UserPack *pack, uint8_t *buffer)
{
	uint8_t i = 0;
	buffer[i++] = pack->FCmd;
	if (pack->FCmd != UserPack_Cmd_Error)
		buffer[i++] = pack->SCmd._raw;
	else
		buffer[i++] = (uint8_t)pack->SCmd.cmd;
	buffer[i++] = pack->TotalSize;
	for (uint8_t j = 0; j < pack->TotalSize; ++j)
		buffer[i++] = pack->Data[j];
	return i;
}



void UserPack_ToStruct(UserPack *pack, const uint8_t *buffer)
{
	uint8_t i = 0;
	pack->FCmd = buffer[i++];
	if (pack->FCmd != UserPack_Cmd_Error)
		pack->SCmd._raw = buffer[i++];
	else
		pack->SCmd.cmd = (UserPack_FCommand)buffer[i++];
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
	pack->FCmd = 0;
	pack->SCmd.cmd = (UserPack_FCommand)0;
	pack->TotalSize = 0;
}


