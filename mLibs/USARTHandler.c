#include "USARTHandler.h"
#include "CheckSum.h"
#include "mUSART.h"


uint16_str crc;


void USARTHandler_Initialization(void)
{
	USART_Initialization();
	CheckSum_Initialization();
}



USARTHandler_RESULT USARTHandler_Send(const UserPack *pack)
{
	uint8_t buffer[UserPack_MAX_PACK_BYTE];
	uint8_t buf_size = UserPack_ToBytes(pack, buffer);
	uint16_str crc;
	crc.d = CheckSum_GetCRC16(buffer, buf_size);
	buffer[buf_size++] = crc.b[0];
	buffer[buf_size++] = crc.b[1];

	USART_SendBuffer(buffer, buf_size);

	return USARTHandler_SUCCESS;
}



USARTHandler_RESULT USARTHandler_Receive(UserPack *pack)
{
	uint8_t buffer[UserPack_MAX_PACK_BYTE];
	uint8_t it = 0, cnt;

	UserPack_Reset(pack);
	
	cnt = UserPack_SERVICE_INFO_SIZE;
	while (cnt--) {
		buffer[it] = USART_GetByte();
		if (USART_GetErrorStatus() == USART_OVERFLOW_RESET) {
			if (it == UserPack_TOTAL_SIZE_OFFSET) {
				pack->TotalSize = buffer[it];
				cnt += pack->TotalSize;
			}
			it++;
		} else {
			USART_ResetErrorStatus();
			buffer[0] = UserPack_Cmd_Status;
			buffer[1] = UserPack_STATUS_Distance;
			buffer[2] = it;
			USART_SendBuffer(buffer, 3);
			return USARTHandler_ERROR;
		}
	}
	
	if (buffer[0] == UserPack_Cmd_Status)
		return USARTHandler_ERROR;
			
	crc.d = CheckSum_GetCRC16(buffer, it - 2);
	if (crc.b[0] == buffer[it - 2] && crc.b[1] == buffer[it - 1]) {
		UserPack_ToStruct(pack, buffer);
		return USARTHandler_SUCCESS;
	}	

	return USARTHandler_ERROR;
}



uint8_t USARTHandler_isAvailableToReceive(void)
{
	return (USART_GetRxStatus() != USART_RX_EMPTY)? 1 : 0;
}

