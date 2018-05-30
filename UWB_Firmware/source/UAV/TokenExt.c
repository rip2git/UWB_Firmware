
//#include "Debugger.h"
#include "TokenExt.h"
#include "Transceiver.h"



#define TokenExt_BUFFER_SIZE			(TokenExt_PAYLOAD_MAX_SIZE + TokenExt_SERVICE_INFO)



typedef union {
	struct {
		uint8_t RootID 	: 7;
		uint8_t Color 	: 1;
	};
	uint8_t val;
} TokenExt_TokenID;



static uint8_t _TokenExt_commonBuffer[TokenExt_BUFFER_SIZE];



static uint8_t _TokenExt_FillCommonBuffer(
		MACHeader_Typedef *header,
		const TokenExt_TokenID *tokenID,
		const uint8_t *payload,
		uint8_t payload_size);



static uint8_t _TokenExt_MAX_ID = 2;
static uint8_t _TokenExt_timeSlotDurationMs = 1;
static TokenExt_TokenID _TokenExt_currentTokenID;
static uint8_t _TokenExt_selfID; // actually unused
static uint8_t _TokenExt_previousOwnerID;
static TokenExt_BOOL _TokenExt_inProcess; // actually unused
static TokenExt_BOOL _TokenExt_TokenExtIsCaptured;



TokenExt_RESULT TokenExt_Transfer(MACHeader_Typedef *header, const uint8_t *payload, uint8_t payload_size)
{
	Transceiver_RESULT TrRes;
	Transceiver_TxConfig tx_config;

	if (_TokenExt_TokenExtIsCaptured == TokenExt_FALSE)
		return TokenExt_FAIL;

	_TokenExt_TokenExtIsCaptured = TokenExt_FALSE;
	_TokenExt_inProcess = TokenExt_FALSE;

	header->DestinationID = MACFrame_BROADCAST_ID;
	header->Flags = (MACFrame_Flags_TOKEN | MACFrame_Flags_SYN);

	tx_config.tx_buffer_size = _TokenExt_FillCommonBuffer(header, &_TokenExt_currentTokenID, payload, payload_size);
	tx_config.tx_buffer = _TokenExt_commonBuffer;
	tx_config.tx_delay = 0;
	tx_config.ranging = 0;
	tx_config.rx_aftertx_delay = 50;
	tx_config.rx_timeout = _TokenExt_timeSlotDurationMs * (_TokenExt_MAX_ID+2) * 1000;
	tx_config.rx_buffer = _TokenExt_commonBuffer;
	tx_config.rx_buffer_size = TokenExt_BUFFER_SIZE;

	TrRes = Transceiver_Transmit( &tx_config );
	if (TrRes == Transceiver_RXFCG) {
		header->SequenceNumber++;

		if (_TokenExt_commonBuffer[MACFrame_FLAGS_OFFSET] ==
				(MACFrame_Flags_TOKEN | MACFrame_Flags_SYN | MACFrame_Flags_ACK) &&
			_TokenExt_commonBuffer[TokenExt_TOKEN_ID_OFFSET] == _TokenExt_currentTokenID.val)
		{
			header->Flags = (MACFrame_Flags_SYN | MACFrame_Flags_ACK);

			tx_config.tx_buffer_size = _TokenExt_FillCommonBuffer( header, &_TokenExt_currentTokenID, 0, 0);
			tx_config.tx_buffer = _TokenExt_commonBuffer;
			tx_config.tx_delay = 0;
			tx_config.ranging = 0;
			tx_config.rx_aftertx_delay = 0;
			tx_config.rx_timeout = 0;
			tx_config.rx_buffer = 0;
			tx_config.rx_buffer_size = 0;

			TrRes = Transceiver_Transmit( &tx_config );
			if (TrRes == Transceiver_TXFRS) {
				header->SequenceNumber++;
				_TokenExt_inProcess = TokenExt_TRUE;
				return TokenExt_SUCCESS;
			}
		}
	} else if (TrRes == Transceiver_RXRFTO) {
		if (_TokenExt_currentTokenID.RootID == (uint8_t)header->SourceID) {
			_TokenExt_currentTokenID.Color = !_TokenExt_currentTokenID.Color;

			_TokenExt_TokenExtIsCaptured = TokenExt_TRUE;
			return TokenExt_SWITCHCOLOR;

		} else { // operation return
			header->DestinationID = _TokenExt_previousOwnerID;
			header->Flags = (MACFrame_Flags_TOKEN | MACFrame_Flags_RET);

			tx_config.tx_buffer_size = _TokenExt_FillCommonBuffer(header, &_TokenExt_currentTokenID, 0, 0);
			tx_config.tx_buffer = _TokenExt_commonBuffer;
			tx_config.tx_delay = 0;
			tx_config.ranging = 0;
			tx_config.rx_aftertx_delay = 0;
			tx_config.rx_timeout = 0;
			tx_config.rx_buffer = 0;
			tx_config.rx_buffer_size = 0;

			TrRes = Transceiver_Transmit( &tx_config );
			if (TrRes == Transceiver_TXFRS) {
				header->SequenceNumber++;
				return TokenExt_SUCCESS;
			}
		}
	}

	return TokenExt_FAIL;
}



TokenExt_RESULT TokenExt_Receipt(MACHeader_Typedef *header, const uint8_t *rx_buffer)
{
	Transceiver_RESULT TrRes;
	Transceiver_TxConfig tx_config;	
	Transceiver_RxConfig rx_config;
	TokenExt_TokenID tmpTokID;
	
	_TokenExt_TokenExtIsCaptured = TokenExt_FALSE;

	tmpTokID.val = rx_buffer[TokenExt_TOKEN_ID_OFFSET];
	if (tmpTokID.val == _TokenExt_currentTokenID.val)
		return TokenExt_FAIL;

	rx_config.rx_buffer = _TokenExt_commonBuffer;
	rx_config.rx_buffer_size = TokenExt_BUFFER_SIZE;
	rx_config.rx_timeout = ((uint8_t)header->SourceID + 1) * _TokenExt_timeSlotDurationMs * 1000;
	rx_config.rx_delay = 0;
	rx_config.rx_interrupt = 0;

	TrRes = Transceiver_Receive( &rx_config );
	if (TrRes == Transceiver_RXFCG) {
		// todo analisys for token reset sending
		return TokenExt_FAIL;
	}

	header->DestinationID = MACFrame_BROADCAST_ID;
	header->Flags = (MACFrame_Flags_TOKEN | MACFrame_Flags_SYN | MACFrame_Flags_ACK);
	
	tx_config.tx_buffer_size = _TokenExt_FillCommonBuffer(header, &tmpTokID, 0, 0);
	tx_config.tx_buffer = _TokenExt_commonBuffer;
	tx_config.tx_delay = 0;
	tx_config.ranging = 0;
	tx_config.rx_aftertx_delay = 50;
	tx_config.rx_timeout = _TokenExt_timeSlotDurationMs * 1000;
	tx_config.rx_buffer = _TokenExt_commonBuffer;
	tx_config.rx_buffer_size = TokenExt_BUFFER_SIZE;
	
	TrRes = Transceiver_Transmit( &tx_config );
	if (TrRes == Transceiver_RXFCG) {
		header->SequenceNumber++;
		if (_TokenExt_commonBuffer[MACFrame_FLAGS_OFFSET] ==
				(MACFrame_Flags_SYN | MACFrame_Flags_ACK) &&
			_TokenExt_commonBuffer[TokenExt_TOKEN_ID_OFFSET] == tmpTokID.val)
		{
			_TokenExt_previousOwnerID = (uint8_t)_TokenExt_commonBuffer[MACFrame_SOURCE_ADDRESS_OFFSET];
			_TokenExt_currentTokenID.val = tmpTokID.val;
			_TokenExt_TokenExtIsCaptured = TokenExt_TRUE;
			return TokenExt_SUCCESS;
		}
	}
	return TokenExt_FAIL;
}



TokenExt_RESULT TokenExt_GetReturnedToken(const uint8_t *rx_buffer)
{
	if (_TokenExt_currentTokenID.val == rx_buffer[TokenExt_TOKEN_ID_OFFSET]) {
		_TokenExt_TokenExtIsCaptured = TokenExt_TRUE;
		_TokenExt_inProcess = TokenExt_FALSE;
		return TokenExt_SUCCESS;
	} else {
		_TokenExt_TokenExtIsCaptured = TokenExt_FALSE;
		_TokenExt_currentTokenID.val = rx_buffer[TokenExt_TOKEN_ID_OFFSET];
		return TokenExt_FAIL;
	}
}



TokenExt_RESULT TokenExt_Generate(MACHeader_Typedef *header)
{
	Transceiver_RESULT TrRes;
	Transceiver_TxConfig tx_config;
	Transceiver_RxConfig rx_config;

	header->Flags = (MACFrame_Flags_SYN | MACFrame_Flags_ACK);

	rx_config.rx_buffer = _TokenExt_commonBuffer;
	rx_config.rx_buffer_size = TokenExt_BUFFER_SIZE;
	rx_config.rx_timeout = _TokenExt_timeSlotDurationMs * 2 * 1000;
	rx_config.rx_delay = 0;
	rx_config.rx_interrupt = 0;

	TrRes = Transceiver_Receive( &rx_config );
	if (TrRes == Transceiver_RXFCG) {
		return TokenExt_FAIL;
	}

	_TokenExt_previousOwnerID = _TokenExt_selfID;
	_TokenExt_currentTokenID.RootID = (uint8_t)header->SourceID;
	_TokenExt_currentTokenID.Color = 0;

	tx_config.tx_buffer_size = _TokenExt_FillCommonBuffer(header, &_TokenExt_currentTokenID, 0, 0);
	tx_config.tx_buffer = _TokenExt_commonBuffer;
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



void TokenExt_Reset()
{
	_TokenExt_TokenExtIsCaptured = TokenExt_FALSE;
}



static uint8_t _TokenExt_FillCommonBuffer(
		MACHeader_Typedef *header,
		const TokenExt_TokenID *tokenID,
		const uint8_t *payload,
		uint8_t payload_size)
{
	uint8_t i = 0;
	// header
	_TokenExt_commonBuffer[i++] = (uint8_t)(header->FrameControl);
	_TokenExt_commonBuffer[i++] = (uint8_t)(header->FrameControl >> 8);
	_TokenExt_commonBuffer[i++] = (uint8_t)(header->SequenceNumber);
	_TokenExt_commonBuffer[i++] = (uint8_t)(header->PAN_ID);
	_TokenExt_commonBuffer[i++] = (uint8_t)(header->PAN_ID >> 8);
	_TokenExt_commonBuffer[i++] = (uint8_t)(header->DestinationID);
	_TokenExt_commonBuffer[i++] = (uint8_t)(header->DestinationID >> 8);
	_TokenExt_commonBuffer[i++] = (uint8_t)(header->SourceID);
	_TokenExt_commonBuffer[i++] = (uint8_t)(header->SourceID >> 8);
	_TokenExt_commonBuffer[i++] = (uint8_t)(header->Flags);
	// token id
	_TokenExt_commonBuffer[i++] = (uint8_t)tokenID->val;
	// payload
	for (uint8_t j = 0; j < payload_size; ++j)
		_TokenExt_commonBuffer[i++] = payload[j];
	// FCS
	_TokenExt_commonBuffer[i++] = 0;
	_TokenExt_commonBuffer[i++] = 0;
	return i;
}



void TokenExt_Initialization(uint8_t selfID, uint8_t maxID, uint8_t timeSlotDurationMs)
{
	_TokenExt_selfID = selfID;
	_TokenExt_MAX_ID = maxID;
	_TokenExt_timeSlotDurationMs = timeSlotDurationMs;
	_TokenExt_inProcess = TokenExt_FALSE;
}


