
#include "Token.h"
#include "Transceiver.h"



#define Token_TIMESLOT_DURATION_MS		5

//#define Token_CONTENT_MASK 				0x3F
#define Token_CONTENT_SIZE				0

#define Token_BUFFER_SIZE				(MACFrame_HEADER_SIZE + Token_CONTENT_SIZE + 2)  // + 2 fcs
#define Token_CONTENT_OFFSET			10



static uint8_t _Token_buffer[Token_BUFFER_SIZE];



//static uint8_t _Token_GetNextID();
static void _Token_ConstructBuffer(MACHeader_Typedef *header);
static uint8_t _Token_GetTimeSlot(uint8_t nextID, uint8_t selfID);



//static uint8_t _Token_content;
static uint8_t _Token_MAX_ID;
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
	tx_config.rx_aftertx_delay = 100;
	tx_config.rx_timeout = Token_TIMESLOT_DURATION_MS * 10 * 1000;
	tx_config.rx_buffer = _Token_buffer;
	tx_config.rx_buffer_size = Token_BUFFER_SIZE;	
	
	TrRes = Transceiver_Transmit( &tx_config );	
	
	if (TrRes == Transceiver_RXFCG) {		
		header->SequenceNumber++;
		return Token_SUCCESS; 
	}	
	
	return Token_FAIL; 
}



Token_RESULT Token_Receipt(MACHeader_Typedef *header, const uint8_t *buffer)
{
	//uint8_t nextID;
	uint8_t otherTokenBuffer[Token_BUFFER_SIZE];
	uint8_t otherTokenTimeout;
	Transceiver_RESULT TrRes;
	Transceiver_TxConfig tx_config;	
	Transceiver_RxConfig rx_config;	
	
	//_Token_content = buffer[Token_CONTENT_OFFSET];
	//_Token_content |= (uint16_t)(buffer[Token_CONTENT_OFFSET] << 8);	
	//nextID = _Token_GetNextID(_Token_content);
	
	header->Flags = MACFrame_Flags_TOKEN | MACFrame_Flags_ACK;
	otherTokenTimeout = _Token_GetTimeSlot(buffer[MACFrame_SOURCE_ADDRESS_OFFSET] + 1, header->SourceID) * Token_TIMESLOT_DURATION_MS * 1000;
	
	_Token_ConstructBuffer(header);
	
	tx_config.tx_buffer = _Token_buffer;
	tx_config.tx_buffer_size = Token_BUFFER_SIZE;
	tx_config.tx_delay = 0;
	tx_config.ranging = 0;
	tx_config.rx_aftertx_delay = 0;
	tx_config.rx_timeout = 0;
	tx_config.rx_buffer = 0;
	tx_config.rx_buffer_size = 0;
	
	
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



static uint8_t _Token_GetTimeSlot(uint8_t nextID, uint8_t selfID)
{
	int res = nextID - selfID;
	if (nextID > _Token_MAX_ID)
		nextID = 1;
	
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
	//_Token_content &= Token_CONTENT_MASK;
	//_Token_buffer[i++] = (uint8_t)(_Token_content);
	//_Token_buffer[i++] = (uint8_t)(_Token_content >> 8);
	// FCS
	_Token_buffer[i++] = 0;
	_Token_buffer[i++] = 0;
}



void Token_SetMaxID(uint8_t ID)
{
	_Token_MAX_ID = ID;
}



/*static uint8_t _Token_GetNextID()
{
	uint8_t ID = 0;
	
	while ( ((uint16_t)(1 << ID) & _Token_content) != 0 )
		ID++;
	
	return ID + 1;	
}*/

