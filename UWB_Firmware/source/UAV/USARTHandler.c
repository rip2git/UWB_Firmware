#include "USARTHandler.h"
#include "CheckSum.h"
#include "mUSART_DMA.h"
#include <string.h>



static void _USARTHandler_USART_cb(void);



static uint8_t *pRxUSARTBuf;
static uint8_t _USARTHandler_TXbuffer[UserPack_PACK_SIZE+4+1]; // +flag +1 crc
static uint8_t _USARTHandler_RXbuffer[USARTx_RX_BUFFER_SIZE]; // +flag +1 crc



static const uint8_t RX_BUFFER_CRC_OFFSET = UserPack_PACK_SIZE+4-2; // +flag -2 dist
static const uint8_t TX_BUFFER_SIZE = sizeof(_USARTHandler_TXbuffer);
static const uint8_t TX_BUFFER_CRC_OFFSET = sizeof(_USARTHandler_TXbuffer)-1;



void USARTHandler_Initialization(void)
{
	USART_Initialization();
	CheckSum_Initialization();
	USART_SetTRXMode(USART_TXWAIT);
	USART_SetCallBacks(_USARTHandler_USART_cb, _USARTHandler_USART_cb);
	
	memset(_USARTHandler_TXbuffer, USARTHANDLER_FLAG_VAL, 4); // flag

	pRxUSARTBuf = USART_GetRxBuffer();
	USART_StartRead(0, USARTx_RX_BUFFER_SIZE); // without autocopying
}



USARTHandler_RESULT USARTHandler_Send(const UserPack *pack)
{	
	if (!pack)
		return USARTHandler_ERROR;

	memcpy(_USARTHandler_TXbuffer+4, pack, UserPack_PACK_SIZE);
	_USARTHandler_TXbuffer[ TX_BUFFER_CRC_OFFSET ] = CheckSum_GetCRC8(_USARTHandler_TXbuffer, TX_BUFFER_CRC_OFFSET);

	return USART_SendBuffer(_USARTHandler_TXbuffer, TX_BUFFER_SIZE) == USART_SUCCESS?
			USARTHandler_SUCCESS : USARTHandler_ERROR;
}



USARTHandler_BOOL USARTHandler_Receive(UserPack *pack)
{
	if (USART_ErrorControl() == USART_FAIL) {
		USART_StartRead(0, USARTx_RX_BUFFER_SIZE);
	} else {
		if (!pack) return USARTHandler_FALSE;

		memcpy(_USARTHandler_RXbuffer, pRxUSARTBuf, USARTx_RX_BUFFER_SIZE);
		for (int i = 0; i < USARTx_RX_BUFFER_SIZE; ++i) {
			if (_USARTHandler_RXbuffer[i] == USARTHANDLER_FLAG_VAL) {
				if (
						_USARTHandler_RXbuffer[i+1] == USARTHANDLER_FLAG_VAL &&
						_USARTHandler_RXbuffer[i+2] == USARTHANDLER_FLAG_VAL &&
						_USARTHandler_RXbuffer[i+3] == USARTHANDLER_FLAG_VAL
				) {
					uint8_t *it = _USARTHandler_RXbuffer+i;
					uint8_t crc = CheckSum_GetCRC8(it, RX_BUFFER_CRC_OFFSET);
					if (crc == it[RX_BUFFER_CRC_OFFSET]) {
						memset(pRxUSARTBuf, 0, USARTx_RX_BUFFER_SIZE);
						USART_ForceReadEnd();
						USART_StartRead(0, USARTx_RX_BUFFER_SIZE);
						memcpy(pack, it+4, UserPack_PACK_SIZE-2); // -2 dist
						pack->distance = 0;
						return USARTHandler_SUCCESS;
					} else {
						i += 3;
					}
				} else {
					i += 3;
				}
			}
		}
	}
	return USARTHandler_FALSE;
}



static void _USARTHandler_USART_cb(void)
{
	// reload full buffer
	memset(pRxUSARTBuf, 0, USARTx_RX_BUFFER_SIZE);
	USART_StartRead(0, USARTx_RX_BUFFER_SIZE);
}
