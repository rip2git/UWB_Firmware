
#include "Token.h"
#include "Transceiver.h"



#define Token_BUFFER_SIZE				(MACFrame_HEADER_SIZE + 2)  // + 2 fcs



static uint8_t _Token_buffer[Token_BUFFER_SIZE];



static void _Token_ConstructBuffer(MACHeader_Typedef *header);
static uint8_t _Token_GetTimeSlot(uint8_t nextID, uint8_t selfID);



static uint8_t _Token_MAX_ID = 2;
static uint8_t _Token_timeSlotDurationMs = 1;
static Token_BOOL _Token_tokenIsCaptured;



Token_RESULT Token_Transfer(MACHeader_Typedef *header)
{
	Transceiver_RESULT TrRes;
	Transceiver_TxConfig tx_config;	
	
	_Token_tokenIsCaptured = Token_FALSE;
	
	header->Flags = MACFrame_Flags_TOKEN;
	
	_Token_ConstructBuffer(header);
	
	tx_config.tx_buffer = _Token_buffer;
	tx_config.tx_buffer_size = Token_BUFFER_SIZE;
	tx_config.tx_delay = 0;
	tx_config.ranging = 0;
	tx_config.rx_aftertx_delay = 50;
	tx_config.rx_timeout = _Token_timeSlotDurationMs * _Token_MAX_ID * 1000;
	tx_config.rx_buffer = _Token_buffer;
	tx_config.rx_buffer_size = Token_BUFFER_SIZE;	
	
	TrRes = Transceiver_Transmit( &tx_config );	
	
	if (TrRes == Transceiver_RXFCG) {		
		header->SequenceNumber++;
		return Token_SUCCESS; 
	}	
	
	return Token_FAIL; 
}



Token_RESULT Token_Receipt(MACHeader_Typedef *header, uint8_t tokenOwnerID)
{
	uint8_t otherTokenBuffer[Token_BUFFER_SIZE];
	uint32_t otherTokenTimeout;
	Transceiver_RESULT TrRes;
	Transceiver_TxConfig tx_config;	
	Transceiver_RxConfig rx_config;			
	
	otherTokenTimeout = _Token_GetTimeSlot(tokenOwnerID, header->SourceID) * _Token_timeSlotDurationMs * 1000;
	
	rx_config.rx_buffer = otherTokenBuffer;
	rx_config.rx_buffer_size = Token_BUFFER_SIZE;
	rx_config.rx_timeout = otherTokenTimeout;
	rx_config.rx_delay = 0;
	rx_config.rx_interrupt = 0;
	
	TrRes = Transceiver_Receive( &rx_config );
			
	if (TrRes == Transceiver_RXFCG) {
		_Token_tokenIsCaptured = Token_FALSE;
		return Token_FAIL;
	}
	
	header->Flags = (MACFrame_Flags_TOKEN | MACFrame_Flags_ACK);
	
	_Token_ConstructBuffer(header);
	
	tx_config.tx_buffer = _Token_buffer;
	tx_config.tx_buffer_size = Token_BUFFER_SIZE;
	tx_config.tx_delay = 0;
	tx_config.ranging = 0;
	tx_config.rx_aftertx_delay = 0;
	tx_config.rx_timeout = 0;
	tx_config.rx_buffer = 0;
	tx_config.rx_buffer_size = 0;
	
	TrRes = Transceiver_Transmit( &tx_config );	
	
	if (TrRes == Transceiver_TXFRS) {
		header->SequenceNumber++;
		_Token_tokenIsCaptured = Token_TRUE;
		return Token_SUCCESS; 
	}
	return Token_FAIL;
}



Token_RESULT Token_ImmediateReceipt(MACHeader_Typedef *header)
{
	Transceiver_RESULT TrRes;
	Transceiver_TxConfig tx_config;	
		
	header->Flags = MACFrame_Flags_TOKEN | MACFrame_Flags_ACK;
	
	_Token_ConstructBuffer(header);
	
	tx_config.tx_buffer = _Token_buffer;
	tx_config.tx_buffer_size = Token_BUFFER_SIZE;
	tx_config.tx_delay = 0;
	tx_config.ranging = 0;
	tx_config.rx_aftertx_delay = 0;
	tx_config.rx_timeout = 0;
	tx_config.rx_buffer = 0;
	tx_config.rx_buffer_size = 0;	
	
	TrRes = Transceiver_Transmit( &tx_config );	
	
	if (TrRes == Transceiver_TXFRS) {
		header->SequenceNumber++;
		_Token_tokenIsCaptured = Token_TRUE;
		return Token_SUCCESS; 
	}
	return Token_FAIL;
}



Token_RESULT Token_Generate(MACHeader_Typedef *header)
{
	Transceiver_RESULT TrRes;
	Transceiver_TxConfig tx_config;	
	
	header->Flags = MACFrame_Flags_TOKEN | MACFrame_Flags_ACK;
		
	_Token_ConstructBuffer(header);
	
	tx_config.tx_buffer = _Token_buffer;
	tx_config.tx_buffer_size = Token_BUFFER_SIZE;
	tx_config.tx_delay = 0;
	tx_config.ranging = 0;
	tx_config.rx_aftertx_delay = 0;
	tx_config.rx_timeout = 0;
	tx_config.rx_buffer = 0;
	tx_config.rx_buffer_size = 0;	
	
	TrRes = Transceiver_Transmit( &tx_config );	
	
	if (TrRes == Transceiver_TXFRS) {
		header->SequenceNumber++;
		_Token_tokenIsCaptured = Token_TRUE;
		return Token_SUCCESS; 
	}
	return Token_FAIL;
}



Token_BOOL Token_isCaptured()
{
	return _Token_tokenIsCaptured;
}



static uint8_t _Token_GetTimeSlot(uint8_t tokenOwnerID, uint8_t selfID)
{
	int res = tokenOwnerID - selfID;
	
	if (res < 0) {
		res = -res;
	} else {
		res = _Token_MAX_ID - res;
	}
	
	return (uint8_t)res;
}



static void _Token_ConstructBuffer(MACHeader_Typedef *header)
{
	uint8_t i = 0;
	// header
	_Token_buffer[i++] = (uint8_t)(header->FrameControl);
	_Token_buffer[i++] = (uint8_t)(header->FrameControl >> 8);
	_Token_buffer[i++] = (uint8_t)(header->SequenceNumber);
	_Token_buffer[i++] = (uint8_t)(header->PAN_ID);
	_Token_buffer[i++] = (uint8_t)(header->PAN_ID >> 8);
	_Token_buffer[i++] = (uint8_t)(header->DestinationID);
	_Token_buffer[i++] = (uint8_t)(header->DestinationID >> 8);
	_Token_buffer[i++] = (uint8_t)(header->SourceID);
	_Token_buffer[i++] = (uint8_t)(header->SourceID >> 8);
	_Token_buffer[i++] = (uint8_t)(header->Flags);	
	// payload
	// FCS
	_Token_buffer[i++] = 0;
	_Token_buffer[i++] = 0;
}



void Token_Initialization(uint8_t ID, uint8_t timeSlotDurationMs)
{
	_Token_MAX_ID = ID;
	_Token_timeSlotDurationMs = timeSlotDurationMs;
}


