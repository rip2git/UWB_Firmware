#include "USARTHandler.h"
#include "CheckSum.h"
#include "mUSART_DMA.h"
#include <string.h>


static USARTHandler_BOOL _USARTHandler_isReadyToReturnData;



static void _USARTHandler_USART_scb(void);
static void _USARTHandler_USART_fcb(void);



static uint8_t _USARTHandler_RXbuffer[UserPack_PACK_SIZE-2+1]; // -2 dist +1 crc
static uint8_t _USARTHandler_TXbuffer[UserPack_PACK_SIZE+1]; // +1 crc
static uint8_t _USARTHandler_UserRXbuffer[sizeof(_USARTHandler_RXbuffer)];



static const uint8_t RX_BUFFER_CRC_OFFSET = sizeof(_USARTHandler_RXbuffer)-1;
static const uint8_t RX_BUFFER_SIZE = sizeof(_USARTHandler_RXbuffer);
static const uint8_t TX_BUFFER_SIZE = sizeof(_USARTHandler_TXbuffer);



void USARTHandler_Initialization(void)
{
	USART_Initialization();
	CheckSum_Initialization();
	USART_SetTRXMode(USART_TXWAIT);
	USART_SetCallBacks(_USARTHandler_USART_scb, _USARTHandler_USART_fcb);
	
	_USARTHandler_isReadyToReturnData = USARTHandler_FALSE;

	USART_StartRead(_USARTHandler_RXbuffer, RX_BUFFER_SIZE);
}



USARTHandler_RESULT USARTHandler_Send(const UserPack *pack)
{	
	if (!pack)
		return USARTHandler_ERROR;

	memcpy(_USARTHandler_TXbuffer, pack, UserPack_PACK_SIZE);
	_USARTHandler_TXbuffer[ UserPack_PACK_SIZE ] = CheckSum_GetCRC8(_USARTHandler_TXbuffer, UserPack_PACK_SIZE);

	return USART_SendBuffer(_USARTHandler_TXbuffer, TX_BUFFER_SIZE) == USART_SUCCESS?
			USARTHandler_SUCCESS : USARTHandler_ERROR;
}



USARTHandler_RESULT USARTHandler_Receive(UserPack *pack)
{
	USARTHandler_RESULT res = USARTHandler_ERROR;

	_USARTHandler_isReadyToReturnData = USARTHandler_FALSE;

	uint8_t crc = CheckSum_GetCRC8(_USARTHandler_RXbuffer, RX_BUFFER_CRC_OFFSET);
	if (pack && crc == _USARTHandler_RXbuffer[ RX_BUFFER_CRC_OFFSET ]) {
		memcpy(pack, _USARTHandler_RXbuffer, UserPack_PACK_SIZE-2); // -2 dist
		pack->distance = 0;
		res = USARTHandler_SUCCESS;
	}

	return res;
}



static void _USARTHandler_USART_scb(void)
{
	_USARTHandler_isReadyToReturnData = USARTHandler_TRUE;
	USART_StartRead(_USARTHandler_RXbuffer, RX_BUFFER_SIZE);
}



static void _USARTHandler_USART_fcb(void)
{
	USART_StartRead(_USARTHandler_RXbuffer, RX_BUFFER_SIZE);
}



USARTHandler_BOOL USARTHandler_isAvailableToReceive(void)
{
	if (USART_ErrorControl() == USART_FAIL) {
		_USARTHandler_isReadyToReturnData = USARTHandler_FALSE;
		USART_StartRead(_USARTHandler_RXbuffer, RX_BUFFER_SIZE);
	}
	return _USARTHandler_isReadyToReturnData;
}

