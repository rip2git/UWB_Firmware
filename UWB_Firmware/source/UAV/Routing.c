
#include "Routing.h"
#include <string.h>



#include "Debugger.h"



#define _Routing_WORST_LVL			255
#define _Routing_THRESHHOLD_LVL		65.0



#define _Routing_SOURCE_OFFSET	 		(MACFrame_PAYLOAD_OFFSET + 0)
#define _Routing_DESTINATION_OFFSET	 	(MACFrame_PAYLOAD_OFFSET + 1)



typedef struct {
	uint8_t flag;
	uint8_t data;
	uint8_t headerSrc;
	uint8_t headerDst;
	uint8_t routingSrc;
	uint8_t routingDst;
} _Routing_WaitResponseConditions;



static uint8_t _Routing_oldTable_Lvls[Routing_TABLE_SIZE];
static uint8_t _Routing_oldTable_IDs[Routing_TABLE_SIZE];
//
static uint8_t _Routing_newTable_Lvls[Routing_TABLE_SIZE];
static uint8_t _Routing_newTable_IDs[Routing_TABLE_SIZE];


static uint8_t _Routing_deviceID;
static uint8_t _Routing_ACKReceivingTimeOut;
static uint8_t _Routing_transactionSize;
static uint8_t _Routing_trustPacks;
static uint8_t _Routing_repeats;


static uint8_t _Routing_buffer[MACFrame_FRAME_MAX_SIZE];
static Transceiver_TxConfig tx_config;



static void _Routing_Refresh_newTable(const uint8_t *buffer, uint8_t tableOwnerID, uint8_t tableOwnerRxlvl);
static void _Routing_Refresh_Tables();
static uint8_t _Routing_GetIntermediateID(uint8_t destinationID);
static uint8_t _Routing_ConstructBuffer(const MACHeader_Typedef *header, const uint8_t *payload, uint8_t payload_size);
static Routing_RESULT _Routing_WaitResponse(_Routing_WaitResponseConditions *cond, uint8_t *respSize);



Routing_RESULT Routing_SendData(MACHeader_Typedef *header, const uint8_t *payload, uint8_t payload_size)
{
	_Routing_WaitResponseConditions cond;

	uint8_t extremeNodes[] = {
			header->SourceID, 		// start node (this)
			header->DestinationID	// end node
	};
	uint8_t neighboringNodes[] = {
			_Routing_GetIntermediateID( extremeNodes[1] ),	// next node (intermediate)
			0												// previous node
	};

	// intermediate node is available
	if (neighboringNodes[0] == 0xFF) {
		Debugger_SendStr("\4\1 0xFF");
		return Routing_FAIL;
	}

	// send connect request - TOKEN SYN -------------------------------------------------------------------
	{
		header->DestinationID = neighboringNodes[0];
		header->Flags = (MACFrame_Flags_TOKEN | MACFrame_Flags_SYN);

		tx_config.tx_buffer_size = _Routing_ConstructBuffer(header, extremeNodes, 2);
		if ( Transceiver_Transmit( &tx_config ) != Transceiver_TXFRS ) {
			return Routing_FAIL;
		}
		header->SequenceNumber++;
	} // --------------------------------------------------------------------------------------------------

	// waiting for connect response - TOKEN SYN ACK -------------------------------------------------------
	{
		cond.flag = (MACFrame_Flags_TOKEN | MACFrame_Flags_SYN | MACFrame_Flags_ACK);
		cond.data = 0; // not data-message
		cond.headerSrc = neighboringNodes[0]; // from intermediate node
		cond.headerDst = header->SourceID; // to this node
		cond.routingSrc = extremeNodes[1]; // to start node
		cond.routingDst = extremeNodes[0]; // from end node
		if ( _Routing_WaitResponse(&cond, 0) == Routing_FAIL ) {
			Debugger_PushSymBack(4);
			Debugger_PushSymBack(2);
			Debugger_PushArrayBack(tx_config.tx_buffer, tx_config.tx_buffer_size);
			Debugger_SendPreparedBuf();
			return Routing_FAIL;
		}
	} // --------------------------------------------------------------------------------------------------

	//for (int i = 0, j = 0; i < _Routing_transactionSize; ++i, ++j)
	{
		// send data - TOKEN DATA -------------------------------------------------------------------------
		{
			header->DestinationID = neighboringNodes[0];
			header->Flags = (MACFrame_Flags_TOKEN | MACFrame_Flags_DATA);

			tx_config.tx_buffer_size = _Routing_ConstructBuffer(header, payload, payload_size);
			if ( Transceiver_Transmit( &tx_config ) != Transceiver_TXFRS ) {
				return Routing_FAIL;
			}
			header->SequenceNumber++;
		} // ----------------------------------------------------------------------------------------------

		//if ( i == (_Routing_transactionSize - 1) || (j % _Routing_trustPacks) == 0 )
		{
			// waiting for data response - TOKEN DATA ACK -------------------------------------------------
			{
				cond.flag = (MACFrame_Flags_TOKEN | MACFrame_Flags_DATA | MACFrame_Flags_ACK);
				cond.data = 0; // not data-message
				cond.headerSrc = neighboringNodes[0]; // from intermediate node
				cond.headerDst = header->SourceID; // to this node
				cond.routingSrc = extremeNodes[1]; // to start node
				cond.routingDst = extremeNodes[0]; // from end node
				if ( _Routing_WaitResponse(&cond, 0) == Routing_FAIL ) {
					Debugger_PushSymBack(4);
					Debugger_PushSymBack(3);
					Debugger_PushArrayBack(tx_config.tx_buffer, tx_config.tx_buffer_size);
					Debugger_SendPreparedBuf();
					return Routing_FAIL;
				}
			} // ------------------------------------------------------------------------------------------
		}
	}
	Debugger_SendStr("\4\4");

	return Routing_SUCCESS;
}



Routing_RESULT Routing_RecvDataTransferRequest(
	MACHeader_Typedef *header,
	const uint8_t *rx_buffer,
	uint8_t *data_buffer,
	uint8_t *data_buffer_size
)
{
	_Routing_WaitResponseConditions cond;

	uint8_t extremeNodes[] = {
			rx_buffer[_Routing_SOURCE_OFFSET], 		// start node
			rx_buffer[_Routing_DESTINATION_OFFSET]	// end node
	};
	uint8_t rev_extremeNodes[] = {
			extremeNodes[1],	// end node
			extremeNodes[0]		// start node
	};

	uint8_t tmp_data_buffer_size = *data_buffer_size; // keep in mind
	*data_buffer_size = 0; // reset

	// ****************************************************************************************************
	// retransmission *************************************************************************************
	if ( rx_buffer[_Routing_DESTINATION_OFFSET] != (uint8_t)(header->SourceID & 0x00FF) ) {

		uint8_t respSize = 0;
		uint8_t neighboringNodes[] = {
				_Routing_GetIntermediateID( extremeNodes[1] ),	// next node
				rx_buffer[MACFrame_SOURCE_ADDRESS_OFFSET]		// previous node
		};

		// intermediate node is available
		if (neighboringNodes[0] == 0xFF) {
			Debugger_SendStr("\3\1 0xFF");
			return Routing_FAIL;
		}

		// send connect request - TOKEN SYN ---------------------------------------------------------------
		{
			header->DestinationID = neighboringNodes[0]; // to next node
			header->Flags = (MACFrame_Flags_TOKEN | MACFrame_Flags_SYN);

			tx_config.tx_buffer_size = _Routing_ConstructBuffer(header, extremeNodes, 2);
			if ( Transceiver_Transmit( &tx_config ) != Transceiver_TXFRS ) {
				return Routing_FAIL;
			}
			header->SequenceNumber++;
		} // ----------------------------------------------------------------------------------------------

		// waiting for connect response - TOKEN SYN ACK ---------------------------------------------------
		{
			cond.flag = (MACFrame_Flags_TOKEN | MACFrame_Flags_SYN | MACFrame_Flags_ACK);
			cond.data = 0; // not data-message
			cond.headerSrc = neighboringNodes[0]; // from next node
			cond.headerDst = header->SourceID; // to this node
			cond.routingSrc = extremeNodes[1]; // from end node
			cond.routingDst = extremeNodes[0]; // to start node
			if ( _Routing_WaitResponse(&cond, 0) == Routing_FAIL ) {
				Debugger_PushSymBack(3);
				Debugger_PushSymBack(2);
				Debugger_PushArrayBack(tx_config.tx_buffer, tx_config.tx_buffer_size);
				Debugger_SendPreparedBuf();
				return Routing_FAIL;
			}
		} // ----------------------------------------------------------------------------------------------

		// send connect ack - TOKEN SYN ACK ---------------------------------------------------------------
		{
			header->DestinationID = neighboringNodes[1]; // to previous node
			header->Flags = (MACFrame_Flags_TOKEN | MACFrame_Flags_SYN | MACFrame_Flags_ACK);

			tx_config.tx_buffer_size = _Routing_ConstructBuffer(header, rev_extremeNodes, 2);
			if ( Transceiver_Transmit( &tx_config ) != Transceiver_TXFRS ) {
				return Routing_FAIL;
			}
			header->SequenceNumber++;
		} // ----------------------------------------------------------------------------------------------

		//for (int i = 0, j = 0; i < _Routing_transactionSize; ++i, ++j)
		{
			// waiting for data - TOKEN DATA --------------------------------------------------------------
			{
				cond.flag = (MACFrame_Flags_TOKEN | MACFrame_Flags_DATA);
				cond.data = 1; // data-message
				cond.headerSrc = neighboringNodes[1]; // from previous node
				cond.headerDst = header->SourceID; // to this node
				cond.routingSrc = 0; // data massege - without routing src
				cond.routingDst = 0; // data massege - without routing dst
				if ( _Routing_WaitResponse(&cond, &respSize) == Routing_FAIL ) {
					Debugger_PushSymBack(3);
					Debugger_PushSymBack(2);
					Debugger_PushArrayBack(tx_config.tx_buffer, tx_config.tx_buffer_size);
					Debugger_SendPreparedBuf();
					return Routing_FAIL;
				}
			} // ------------------------------------------------------------------------------------------

			// send data - TOKEN DATA ---------------------------------------------------------------------
			{
				header->DestinationID = neighboringNodes[0]; // to next node
				header->Flags = (MACFrame_Flags_TOKEN | MACFrame_Flags_DATA);

				tx_config.tx_buffer_size = _Routing_ConstructBuffer(
						header,
						&(_Routing_buffer[MACFrame_PAYLOAD_OFFSET]),
						respSize - MACFrame_HEADER_SIZE - MACFrame_FCS_SIZE);
				if ( Transceiver_Transmit( &tx_config ) != Transceiver_TXFRS ) {
					return Routing_FAIL;
				}
				header->SequenceNumber++;
			} // ------------------------------------------------------------------------------------------

			//if ( i == (_Routing_transactionSize - 1) || (j % _Routing_trustPacks) == 0 )
			{
				// waiting for data ack - TOKEN DATA ACK --------------------------------------------------
				{
					cond.flag = (MACFrame_Flags_TOKEN | MACFrame_Flags_DATA | MACFrame_Flags_ACK);
					cond.data = 0; // not data-message
					cond.headerSrc = neighboringNodes[0]; // from next node
					cond.headerDst = header->SourceID; // to this node
					cond.routingSrc = extremeNodes[1]; // from end node
					cond.routingDst = extremeNodes[0]; // to start node
					if ( _Routing_WaitResponse(&cond, 0) == Routing_FAIL ) {
						Debugger_PushSymBack(3);
						Debugger_PushSymBack(4);
						Debugger_PushArrayBack(tx_config.tx_buffer, tx_config.tx_buffer_size);
						Debugger_SendPreparedBuf();
						return Routing_FAIL;
					}
				} // --------------------------------------------------------------------------------------

				// send data ack - TOKEN DATA ACK ---------------------------------------------------------
				{
					header->DestinationID = neighboringNodes[1]; // to previous node
					header->Flags = (MACFrame_Flags_TOKEN | MACFrame_Flags_DATA | MACFrame_Flags_ACK);

					tx_config.tx_buffer_size = _Routing_ConstructBuffer(header, rev_extremeNodes, 2);
					if ( Transceiver_Transmit( &tx_config ) != Transceiver_TXFRS ) {
						return Routing_FAIL;
					}
					header->SequenceNumber++;
				} // --------------------------------------------------------------------------------------
			}
		}
		Debugger_SendStr("\3\5");

		return Routing_SUCCESS;
	}
	// ****************************************************************************************************
	// reception ******************************************************************************************
	else {

		uint8_t neighboringNodes[] = {
				0,	// next node
				rx_buffer[MACFrame_SOURCE_ADDRESS_OFFSET]		// previous node
		};

		// send connect ACK - TOKEN SYN ACK ---------------------------------------------------------------
		{
			header->DestinationID = neighboringNodes[1]; // to previous node
			header->Flags = (MACFrame_Flags_TOKEN | MACFrame_Flags_SYN | MACFrame_Flags_ACK);

			tx_config.tx_buffer_size = _Routing_ConstructBuffer(header, rev_extremeNodes, 2);
			if ( Transceiver_Transmit( &tx_config ) != Transceiver_TXFRS )  {
				return Routing_FAIL;
			}
		} // ----------------------------------------------------------------------------------------------

		//for (int i = 0, j = 0; i < _Routing_transactionSize; ++i, ++j)
		{
			// waiting for data - TOKEN DATA --------------------------------------------------------------
			{
				cond.flag = (MACFrame_Flags_TOKEN | MACFrame_Flags_DATA);
				cond.data = 1; // data-message
				cond.headerSrc = neighboringNodes[1]; // from previous node
				cond.headerDst = header->SourceID; // to this node
				cond.routingSrc = 0; // data massege - without routing src
				cond.routingDst = 0; // data massege - without routing dst
				if ( _Routing_WaitResponse(&cond, data_buffer_size) == Routing_FAIL ) {
					Debugger_PushSymBack(3);
					Debugger_PushSymBack(6);
					Debugger_PushArrayBack(tx_config.tx_buffer, tx_config.tx_buffer_size);
					Debugger_SendPreparedBuf();
					return Routing_FAIL;
				}
				*data_buffer_size = (*data_buffer_size > tmp_data_buffer_size)? tmp_data_buffer_size : *data_buffer_size;
				_Routing_buffer[MACFrame_SOURCE_ADDRESS_OFFSET] = extremeNodes[0];
				memcpy(data_buffer, _Routing_buffer, *data_buffer_size);
			} // ------------------------------------------------------------------------------------------

			//if ( i == (_Routing_transactionSize - 1) || (j % _Routing_trustPacks) == 0 )
			{
				// send data ACK - TOKEN DATA ACK ---------------------------------------------------------
				{
					header->DestinationID = neighboringNodes[1]; // to previous node
					header->Flags = (MACFrame_Flags_TOKEN | MACFrame_Flags_DATA | MACFrame_Flags_ACK);

					tx_config.tx_buffer_size = _Routing_ConstructBuffer(header, rev_extremeNodes, 2);
					if ( Transceiver_Transmit( &tx_config ) != Transceiver_TXFRS ) {
						return Routing_FAIL;
					}
				} // --------------------------------------------------------------------------------------
			}
		}
		Debugger_SendStr("\3\7");

		return Routing_SUCCESS;
	}
	// ****************************************************************************************************
	// ****************************************************************************************************
}



void Routing_SendTokenWithTable(MACHeader_Typedef *header)
{
	TokenExt_Transfer( header, _Routing_oldTable_Lvls, Routing_TABLE_SIZE );
}



void Routing_RecvTokenWithTable(MACHeader_Typedef *header, const uint8_t *rx_buffer)
{
	uint8_t rxlvl = Transceiver_GetLevelOfLastReceived();
	TokenExt_Receipt( header, rx_buffer[MACFrame_SOURCE_ADDRESS_OFFSET] );
	_Routing_Refresh_newTable( &(rx_buffer[MACFrame_PAYLOAD_OFFSET]), rx_buffer[MACFrame_SOURCE_ADDRESS_OFFSET], rxlvl );

	if (TokenExt_isCaptured() == TokenExt_TRUE) { // new cycle
		_Routing_Refresh_Tables();
	}
}



void Routing_Initialization(const Routing_InitializationStruct *initializationStruct)
{
	for (uint8_t i = 0; i < Routing_TABLE_SIZE; ++i) {
		_Routing_oldTable_Lvls[i] = _Routing_WORST_LVL;
		_Routing_oldTable_IDs[i] = _Routing_WORST_LVL;

		_Routing_newTable_Lvls[i] = _Routing_WORST_LVL;
		_Routing_newTable_IDs[i] = _Routing_WORST_LVL;
	}

	_Routing_deviceID = initializationStruct->deviceID - 1; // simplifies the logic
	_Routing_ACKReceivingTimeOut = initializationStruct->ACKReceivingTimeOut;
	_Routing_transactionSize = initializationStruct->transactionSize;
	_Routing_trustPacks = initializationStruct->trustPacks + 1; // simplifies the logic
	_Routing_repeats = initializationStruct->repeats;
	BaseTimer_SetPrescaler(SystemCoreClock / 1000); // ms

	tx_config.tx_buffer = _Routing_buffer;
	tx_config.tx_delay = 0;
	tx_config.ranging = 0;
	tx_config.rx_aftertx_delay = 0;
	tx_config.rx_timeout = 0;
	tx_config.rx_buffer = 0;
	tx_config.rx_buffer_size = 0;
}



/*static inline uint8_t _Routing_DefineNewLvl(double lvl)
{
	return log10( pow((lvl), 1.31) + 0.338 ) + 0.66;
}*/



static uint8_t _Routing_DefineNewLvl(uint8_t lvl)
{
	return lvl + 1;
}



static void _Routing_Refresh_newTable(const uint8_t *buffer, uint8_t tableOwnerID, uint8_t tableOwnerRxlvl)
{
	tableOwnerID--;

	Debugger_ConstrAndSendBuf(3, 2, tableOwnerID+1, tableOwnerRxlvl);

	if (tableOwnerRxlvl > _Routing_THRESHHOLD_LVL) {
		for (uint8_t i = 0, tmp; i < Routing_TABLE_SIZE; ++i) {
			if (i != _Routing_deviceID) {
				if (i != tableOwnerID) {
					if (buffer[i] != _Routing_WORST_LVL) {
						tmp = _Routing_DefineNewLvl( buffer[i] );
						if (_Routing_newTable_Lvls[i] >= tmp) {
							_Routing_newTable_Lvls[i] = tmp;
							_Routing_newTable_IDs[i] = tableOwnerID;
						}
					}
				} else {
					_Routing_newTable_Lvls[i] = 1;
					_Routing_newTable_IDs[i] = tableOwnerID;
				}
			}
		}
	}

	/*
	for (uint8_t i = 0, tmp; i < Routing_TABLE_SIZE; ++i) {
		if (i != tableOwnerID) {
			tmp = _Routing_DefineNewLvl(((double)buffer[i] / 100.0) * ((double)tableOwnerRxlvl / 100.0)) * 100.0;
			if (tmp != _Routing_WORST_LVL && _Routing_newTable_Lvls[i] <= tmp) {
				_Routing_newTable_Lvls[i] = tmp;
				_Routing_newTable_IDs[i] = tableOwnerID;
			}
		} else {
			_Routing_newTable_Lvls[i] = tableOwnerRxlvl;
			_Routing_newTable_IDs[i] = tableOwnerID;
		}
	}
	*/
}



static void _Routing_Refresh_Tables()
{
	Debugger_PushSymBack(1);
	Debugger_PushArrayBack(_Routing_oldTable_Lvls, 10);
	Debugger_PushArrayBack(_Routing_oldTable_IDs, 10);
	Debugger_SendPreparedBuf();

	for (uint8_t i = 0; i < Routing_TABLE_SIZE; ++i) {
		_Routing_oldTable_Lvls[i] = _Routing_newTable_Lvls[i];
		_Routing_oldTable_IDs[i] = _Routing_newTable_IDs[i];

		_Routing_newTable_Lvls[i] = _Routing_WORST_LVL;
		_Routing_newTable_IDs[i] = _Routing_WORST_LVL;
	}
}



static uint8_t _Routing_GetIntermediateID(uint8_t destinationID)
{
	destinationID--;
	if (_Routing_newTable_Lvls[destinationID] != _Routing_WORST_LVL)
		return _Routing_newTable_IDs[destinationID] + 1;
	if (_Routing_oldTable_Lvls[destinationID] != _Routing_WORST_LVL)
		return _Routing_oldTable_IDs[destinationID] + 1;
	return 0xFF;
}



static Routing_RESULT _Routing_WaitResponse(_Routing_WaitResponseConditions *cond, uint8_t *respSize)
{
	uint8_t receivingState = 0;
	BaseTimer_Reset();
	BaseTimer_SetPeriod(_Routing_ACKReceivingTimeOut);
	BaseTimer_Enable();
	while (1) {
		if (BaseTimer_GetState() == BaseTimer_SET) {
			BaseTimer_Disable();
			return Routing_FAIL;
		}

		switch (receivingState) {
			case 0: // receiver turn on
			{
				Transceiver_ReceiverOn();
				receivingState = 1; // waiting request
			} break;
			case 1: // get data if it is available
			{
				if (Transceiver_GetReceptionResult() == Transceiver_RXFCG) {
					if (respSize != 0)
						*respSize = Transceiver_GetAvailableData(_Routing_buffer);
					else
						Transceiver_GetAvailableData(_Routing_buffer);
					if (
						_Routing_buffer[MACFrame_FLAGS_OFFSET] == cond->flag &&
						_Routing_buffer[MACFrame_SOURCE_ADDRESS_OFFSET] == cond->headerSrc &&
						_Routing_buffer[MACFrame_DESTINATION_ADDRESS_OFFSET] == cond->headerDst &&
						(cond->data ||
						(_Routing_buffer[_Routing_SOURCE_OFFSET] == cond->routingSrc &&
						_Routing_buffer[_Routing_DESTINATION_OFFSET] == cond->routingDst))
					) {
						BaseTimer_Disable();
						return Routing_SUCCESS;
					} else {
						receivingState = 0; // reset
					}
				}
			} break;
		}
	}
}



static uint8_t _Routing_ConstructBuffer(const MACHeader_Typedef *header, const uint8_t *payload, uint8_t payload_size)
{
	uint8_t i = 0;
	// header
	_Routing_buffer[i++] = (uint8_t)(header->FrameControl);
	_Routing_buffer[i++] = (uint8_t)(header->FrameControl >> 8);
	_Routing_buffer[i++] = (uint8_t)(header->SequenceNumber);
	_Routing_buffer[i++] = (uint8_t)(header->PAN_ID);
	_Routing_buffer[i++] = (uint8_t)(header->PAN_ID >> 8);
	_Routing_buffer[i++] = (uint8_t)(header->DestinationID);
	_Routing_buffer[i++] = (uint8_t)(header->DestinationID >> 8);
	_Routing_buffer[i++] = (uint8_t)(header->SourceID);
	_Routing_buffer[i++] = (uint8_t)(header->SourceID >> 8);
	_Routing_buffer[i++] = (uint8_t)(header->Flags);
	// payload
	for (uint8_t j = 0; j < payload_size; ++j)
		_Routing_buffer[i++] = payload[j];
	// FCS
	_Routing_buffer[i++] = 0;
	_Routing_buffer[i++] = 0;
	return i;
}


