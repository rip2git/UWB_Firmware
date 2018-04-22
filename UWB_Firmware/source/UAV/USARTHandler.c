#include "USARTHandler.h"
#include "CheckSum.h"
#ifndef USART_DMA_VERSION
	#include "mUSART.h"
#else
	#include "mUSART_DMA.h"
#endif



#ifndef USART_DMA_VERSION
#else
static USARTHandler_BOOL _USARTHandler_WaitingSecondPart;	
static USARTHandler_BOOL _USARTHandler_ReadyToReturnData;
static USARTHandler_BOOL _USARTHandler_CallBackRecvPart;



static void _USARTHandler_USART_scb(void);
static void _USARTHandler_USART_fcb(void);
#endif



uint16_str crc;
static uint8_t _USARTHandler_RXbuffer[UserPack_MAX_PACK_BYTE];
static uint8_t _USARTHandler_TXbuffer[UserPack_MAX_PACK_BYTE];



void USARTHandler_Initialization(void)
{
	USART_Initialization();
	CheckSum_Initialization();
#ifndef USART_DMA_VERSION	
#else
	USART_SetTRXMode(USART_NOWAIT);
	USART_SetCallBacks(_USARTHandler_USART_scb, _USARTHandler_USART_fcb);
	
	_USARTHandler_WaitingSecondPart = USARTHandler_FALSE;
	_USARTHandler_ReadyToReturnData = USARTHandler_FALSE;	
	_USARTHandler_CallBackRecvPart = USARTHandler_TRUE;
#endif
}



USARTHandler_RESULT USARTHandler_Send(const UserPack *pack)
{	
	uint8_t buf_size = UserPack_ToBytes(pack, _USARTHandler_TXbuffer);
	crc.d = CheckSum_GetCRC16(_USARTHandler_TXbuffer, buf_size);
	_USARTHandler_TXbuffer[buf_size++] = crc.b[0];
	_USARTHandler_TXbuffer[buf_size++] = crc.b[1];

	USART_SendBuffer(_USARTHandler_TXbuffer, buf_size);

	return USARTHandler_SUCCESS;
}



USARTHandler_RESULT USARTHandler_Receive(UserPack *pack)
{
	uint8_t it = 0;

	UserPack_Reset(pack);
	
#ifndef USART_DMA_VERSION	
	uint8_t cnt;
	
	while (USART_GetRxCount() < UserPack_SERVICE_INFO_SIZE)
		;
	
	cnt = UserPack_SERVICE_INFO_SIZE;
	while (cnt--) {	
		_USARTHandler_RXbuffer[it] = USART_GetByte();
		if (USART_GetErrorStatus() == USART_NO_ERROR) {			
			if (it == UserPack_TOTAL_SIZE_OFFSET) {
				pack->TotalSize = _USARTHandler_RXbuffer[it];
				cnt += pack->TotalSize + UserPack_FCS_SIZE;
			}
			it++;
		} else {			
			USART_ResetErrorStatus();
			USART_ResetRxBuffer();
			_USARTHandler_RXbuffer[0] = UserPack_Cmd_Status;
			_USARTHandler_RXbuffer[1] = UserPack_STATUS_Distance; // ??
			_USARTHandler_RXbuffer[2] = it;
			USART_SendBuffer(_USARTHandler_RXbuffer, 3);
			return USARTHandler_ERROR;
		}
	}
	
#else
	_USARTHandler_WaitingSecondPart = USARTHandler_FALSE;
		
	it = UserPack_SERVICE_INFO_SIZE + UserPack_FCS_SIZE + _USARTHandler_RXbuffer[UserPack_TOTAL_SIZE_OFFSET];
	
#endif	
	
	if (_USARTHandler_RXbuffer[0] == (uint8_t)UserPack_Cmd_Error)
		return USARTHandler_ERROR;
			
	crc.d = CheckSum_GetCRC16(_USARTHandler_RXbuffer, it - 2);
	if (crc.b[0] == _USARTHandler_RXbuffer[it - 2] && crc.b[1] == _USARTHandler_RXbuffer[it - 1]) {
		UserPack_ToStruct(pack, _USARTHandler_RXbuffer);
		return USARTHandler_SUCCESS;
	}	

	return USARTHandler_ERROR;
}



#ifndef USART_DMA_VERSION
#else
static void _USARTHandler_USART_scb(void)
{
	if (_USARTHandler_CallBackRecvPart) { // receive first part
		if (_USARTHandler_RXbuffer[UserPack_TOTAL_SIZE_OFFSET] <= UserPack_MAX_DATA_SIZE) {
			USART_StartRead(
				&(_USARTHandler_RXbuffer[UserPack_TOTAL_SIZE_OFFSET + 1]),
				_USARTHandler_RXbuffer[UserPack_TOTAL_SIZE_OFFSET] + UserPack_FCS_SIZE
			);
			_USARTHandler_CallBackRecvPart = !_USARTHandler_CallBackRecvPart;
		} else {
			_USARTHandler_WaitingSecondPart = USARTHandler_FALSE;
		}		
	} else { // receive second part
		_USARTHandler_ReadyToReturnData = USARTHandler_TRUE;
		_USARTHandler_CallBackRecvPart = !_USARTHandler_CallBackRecvPart;
	}
}



static void _USARTHandler_USART_fcb(void)
{
	_USARTHandler_WaitingSecondPart = USARTHandler_FALSE;
}
#endif



USARTHandler_BOOL USARTHandler_isAvailableToReceive(void)
{
#ifndef USART_DMA_VERSION	
	return (USART_GetRxCounter() >= UserPack_SERVICE_INFO_SIZE)? USARTHandler_TRUE : USARTHandler_FALSE;
#else
	if ( !_USARTHandler_WaitingSecondPart ) {
		USART_StartRead(_USARTHandler_RXbuffer, UserPack_SERVICE_INFO_SIZE);
		_USARTHandler_WaitingSecondPart = USARTHandler_TRUE;  // sets to prevent restarts
		_USARTHandler_ReadyToReturnData = USARTHandler_FALSE;
	}
	if (USART_ErrorControl() == USART_FAIL)
		_USARTHandler_WaitingSecondPart = USARTHandler_FALSE; // resets
	return _USARTHandler_ReadyToReturnData;		
#endif
}

