
#include "TokenExt.h"
#include "Transceiver.h"



#define TokenExt_SERVICE_INFO				(MACFrame_HEADER_SIZE + 2)  // + 2 fcs
#define TokenExt_BUFFER_SIZE				(TokenExt_MAX_PAYLOAD_SIZE + TokenExt_SERVICE_INFO)



static uint8_t _TokenExt_buffer[TokenExt_BUFFER_SIZE];



static uint8_t _TokenExt_ConstructBuffer(MACHeader_Typedef *header, const uint8_t *payload, uint8_t payload_size);
static uint8_t _TokenExt_GetTimeSlot(uint8_t nextID, uint8_t selfID);



static uint8_t _TokenExt_MAX_ID = 2;
static uint8_t _TokenExt_timeSlotDurationMs = 1;
static TokenExt_BOOL _TokenExt_TokenExtIsCaptured;



TokenExt_RESULT TokenExt_Transfer(MACHeader_Typedef *header, const uint8_t *payload, uint8_t payload_size)
{
	Transceiver_RESULT TrRes;
	Transceiver_TxConfig tx_config;

	_TokenExt_TokenExtIsCaptured = TokenExt_FALSE;

	header->Flags |= MACFrame_Flags_TOKEN;

	tx_config.tx_buffer_size = _TokenExt_ConstructBuffer(header, payload, payload_size);
	tx_config.tx_buffer = _TokenExt_buffer;
	tx_config.tx_delay = 0;
	tx_config.ranging = 0;
	tx_config.rx_aftertx_delay = 50;
	tx_config.rx_timeout = _TokenExt_timeSlotDurationMs * _TokenExt_MAX_ID * 1000;
	tx_config.rx_buffer = _TokenExt_buffer;
	tx_config.rx_buffer_size = TokenExt_BUFFER_SIZE;

	TrRes = Transceiver_Transmit( &tx_config );

	if (TrRes == Transceiver_RXFCG) {
		header->SequenceNumber++;
		return TokenExt_SUCCESS;
	}

	return TokenExt_FAIL;
}



TokenExt_RESULT TokenExt_Receipt(MACHeader_Typedef *header, uint8_t tokenOwnerID)
{
	uint8_t otherTokenBuffer[TokenExt_BUFFER_SIZE];
	uint32_t otherTokenTimeout;
	Transceiver_RESULT TrRes;
	Transceiver_TxConfig tx_config;	
	Transceiver_RxConfig rx_config;			
	
	otherTokenTimeout = _TokenExt_GetTimeSlot(tokenOwnerID, header->SourceID) * _TokenExt_timeSlotDurationMs * 1000;
	
	rx_config.rx_buffer = otherTokenBuffer;
	rx_config.rx_buffer_size = TokenExt_BUFFER_SIZE;
	rx_config.rx_timeout = otherTokenTimeout;
	rx_config.rx_delay = 0;
	rx_config.rx_interrupt = 0;
	
	TrRes = Transceiver_Receive( &rx_config );
			
	if (TrRes == Transceiver_RXFCG) {
		_TokenExt_TokenExtIsCaptured = TokenExt_FALSE;
		return TokenExt_FAIL;
	}
	
	header->Flags = (MACFrame_Flags_TOKEN | MACFrame_Flags_ACK);
	
	tx_config.tx_buffer_size = _TokenExt_ConstructBuffer(header, 0, 0);
	tx_config.tx_buffer = _TokenExt_buffer;
	tx_config.tx_delay = 0;
	tx_config.ranging = 0;
	tx_config.rx_aftertx_delay = 0;
	tx_config.rx_timeout = 0;
	tx_config.rx_buffer = 0;
	tx_config.rx_buffer_size = 0;
	
	TrRes = Transceiver_Transmit( &tx_config );	
	
	if (TrRes == Transceiver_TXFRS) {
		header->SequenceNumber++;
		_TokenExt_TokenExtIsCaptured = TokenExt_TRUE;
		return TokenExt_SUCCESS;
	}
	return TokenExt_FAIL;
}



TokenExt_RESULT TokenExt_ImmediateReceipt(MACHeader_Typedef *header)
{
	Transceiver_RESULT TrRes;
	Transceiver_TxConfig tx_config;	
		
	header->Flags = (MACFrame_Flags_TOKEN | MACFrame_Flags_ACK);
	
	tx_config.tx_buffer_size = _TokenExt_ConstructBuffer(header, 0, 0);
	tx_config.tx_buffer = _TokenExt_buffer;
	tx_config.tx_delay = 0;
	tx_config.ranging = 0;
	tx_config.rx_aftertx_delay = 0;
	tx_config.rx_timeout = 0;
	tx_config.rx_buffer = 0;
	tx_config.rx_buffer_size = 0;	
	
	TrRes = Transceiver_Transmit( &tx_config );	
	
	if (TrRes == Transceiver_TXFRS) {
		header->SequenceNumber++;
		_TokenExt_TokenExtIsCaptured = TokenExt_TRUE;
		return TokenExt_SUCCESS;
	}
	return TokenExt_FAIL;
}



TokenExt_BOOL TokenExt_isCaptured()
{
	return _TokenExt_TokenExtIsCaptured;
}



static uint8_t _TokenExt_GetTimeSlot(uint8_t TokenExtOwnerID, uint8_t selfID)
{
	int res = TokenExtOwnerID - selfID;
	
	if (res < 0) {
		res = -res;
	} else {
		res = _TokenExt_MAX_ID - res;
	}
	
	return (uint8_t)res;
}



static uint8_t _TokenExt_ConstructBuffer(MACHeader_Typedef *header, const uint8_t *payload, uint8_t payload_size)
{
	uint8_t i = 0;
	// header
	_TokenExt_buffer[i++] = (uint8_t)(header->FrameControl);
	_TokenExt_buffer[i++] = (uint8_t)(header->FrameControl >> 8);
	_TokenExt_buffer[i++] = (uint8_t)(header->SequenceNumber);
	_TokenExt_buffer[i++] = (uint8_t)(header->PAN_ID);
	_TokenExt_buffer[i++] = (uint8_t)(header->PAN_ID >> 8);
	_TokenExt_buffer[i++] = (uint8_t)(header->DestinationID);
	_TokenExt_buffer[i++] = (uint8_t)(header->DestinationID >> 8);
	_TokenExt_buffer[i++] = (uint8_t)(header->SourceID);
	_TokenExt_buffer[i++] = (uint8_t)(header->SourceID >> 8);
	_TokenExt_buffer[i++] = (uint8_t)(header->Flags);
	// payload
	for (uint8_t j = 0; j < payload_size; ++j)
		_TokenExt_buffer[i++] = payload[j];
	// FCS
	_TokenExt_buffer[i++] = 0;
	_TokenExt_buffer[i++] = 0;
	return i;
}



void TokenExt_Initialization(uint8_t ID, uint8_t timeSlotDurationMs)
{
	_TokenExt_MAX_ID = ID;
	_TokenExt_timeSlotDurationMs = timeSlotDurationMs;
}


