
#include "Routing.h"
#include "Debugger.h"
#include <string.h>



/* todo
 *
 * Можно сделать передачу маркера на основе информации об уровнях сигнала до соседей: накопление таблицы уровней.
 * Тогда маркеры будут передаваться за 1 мс до каждого соседа, при этом операция возврата также будет выполнена
 * за 1 мс. Маркеры можно передавать броадкастом, но включать во фрейм нового владельца маркера и таблицу
 * маршрутизации. Тогда новые устройства, появившиеся на связи, не смогут присоединиться к графу, но будут
 * накапливать как маршрутизацию, так и таблицу уровней. Новые устройства будут включены в граф только по инициации
 * дистанции, когда и будет обновлена таблица уровней ИЛИ ввести дополнителую возможность узнать соседей.
 *
 * */



#define _Routing_WORST_LVL			0xFF



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



/*
 * Positional tables
 * position - id of required device minus 1
 * id in this position - node through which that id could be reached
 * */
typedef union {
	struct {
		uint8_t owner : 4;
		uint8_t hop : 4;
	};
	uint8 _raw;
} _Routing_Unit;

typedef struct {
	_Routing_Unit units[Routing_TABLE_SIZE];
} _Routing_ExternalTable;

typedef struct {
	uint8_t hops[Routing_TABLE_SIZE];
	uint8_t ids[Routing_TABLE_SIZE];
} _Routing_InternalTable;



static _Routing_InternalTable _Routing_oldTable;
static _Routing_InternalTable _Routing_newTable;
static _Routing_ExternalTable _Routing_rxtxTable;



static uint8_t _Routing_deviceID;
static uint8_t _Routing_ACKReceivingTimeOut;
static uint8_t _Routing_THRESHHOLD_LVL;
//static uint8_t _Routing_transactionSize;
//static uint8_t _Routing_trustPacks; // todo  + 1; // simplifies the logic
//static uint8_t _Routing_repeats; // ?????????????????????



static uint8_t _Routing_commonBuffer[MACFrame_FRAME_MAX_SIZE];
static Transceiver_TxConfig tx_config;



static void _Routing_Refresh_newTable(const _Routing_ExternalTable *table, uint8_t tableOwnerID, uint8_t tableOwnerRxlvl);
static void _Routing_RefreshInternalTables();
static uint8_t _Routing_GetIntermediateID(uint8_t destinationID);
static uint8_t _Routing_FillCommonBuffer(
		const MACHeader_Typedef *header,
		const uint8_t *payload,
		uint8_t payload_size);
static Routing_RESULT _Routing_WaitResponse(_Routing_WaitResponseConditions *cond, uint8_t *respSize);
static void _Routing_PopFromTables(uint8_t deviceID);

static int _Routing_BufferToTable(_Routing_ExternalTable *table, const uint8_t *buffer);
static int _Routing_ExternalTableToBuffer(const _Routing_ExternalTable *table, uint8_t *buffer);



Routing_RESULT Routing_SendData(MACHeader_Typedef *header, const uint8_t *payload, uint8_t payload_size)
{
	Routing_RESULT result;
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
	if (neighboringNodes[0] == _Routing_WORST_LVL) {
		Debugger_SendStr("\4\1");
		return Routing_ERROR;
	}

	// send connect request - DATA SYN -------------------------------------------------------------------
	{
		header->DestinationID = neighboringNodes[0];
		header->Flags = (MACFrame_Flags_DATA | MACFrame_Flags_SYN);

		tx_config.tx_buffer_size = _Routing_FillCommonBuffer(header, extremeNodes, 2);
		if ( Transceiver_Transmit( &tx_config ) != Transceiver_TXFRS ) {
			return Routing_ERROR;
		}
		header->SequenceNumber++;
	} // --------------------------------------------------------------------------------------------------

	// waiting for connect response - DATA SYN ACK -------------------------------------------------------
	{
		cond.flag = (MACFrame_Flags_DATA | MACFrame_Flags_SYN | MACFrame_Flags_ACK);
		cond.data = 0; // not data-message
		cond.headerSrc = neighboringNodes[0]; // from intermediate node
		cond.headerDst = header->SourceID; // to this node
		cond.routingSrc = extremeNodes[1]; // to start node
		cond.routingDst = extremeNodes[0]; // from end node
		result = _Routing_WaitResponse(&cond, 0);
		if ( result != Routing_SUCCESS ) {
			Debugger_SendStr("\4\2");
			_Routing_PopFromTables( extremeNodes[1] );
			return result;
		}
	} // --------------------------------------------------------------------------------------------------

	//for (int i = 0, j = 0; i < _Routing_transactionSize; ++i, ++j)
	{
		// send data - DATA -------------------------------------------------------------------------
		{
			header->DestinationID = neighboringNodes[0];
			header->Flags = (MACFrame_Flags_DATA);

			tx_config.tx_buffer_size = _Routing_FillCommonBuffer(header, payload, payload_size);
			if ( Transceiver_Transmit( &tx_config ) != Transceiver_TXFRS ) {
				return Routing_ERROR;
			}
			header->SequenceNumber++;
		} // ----------------------------------------------------------------------------------------------

		//if ( i == (_Routing_transactionSize - 1) || (j % _Routing_trustPacks) == 0 )
		{
			// waiting for data response - DATA ACK -------------------------------------------------
			{
				cond.flag = (MACFrame_Flags_DATA | MACFrame_Flags_ACK);
				cond.data = 0; // not data-message
				cond.headerSrc = neighboringNodes[0]; // from intermediate node
				cond.headerDst = header->SourceID; // to this node
				cond.routingSrc = extremeNodes[1]; // to start node
				cond.routingDst = extremeNodes[0]; // from end node
				result = _Routing_WaitResponse(&cond, 0);
				if ( result != Routing_SUCCESS ) {
					Debugger_SendStr("\4\3");
					_Routing_PopFromTables( extremeNodes[1] );
					return result;
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
	Routing_RESULT result;
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
		if (neighboringNodes[0] == _Routing_WORST_LVL) {
			Debugger_SendStr("\3\1");
			return Routing_ERROR;
		}

		// send connect request - DATA SYN ---------------------------------------------------------------
		{
			header->DestinationID = neighboringNodes[0]; // to next node
			header->Flags = (MACFrame_Flags_DATA | MACFrame_Flags_SYN);

			tx_config.tx_buffer_size = _Routing_FillCommonBuffer(header, extremeNodes, 2);
			if ( Transceiver_Transmit( &tx_config ) != Transceiver_TXFRS ) {
				return Routing_ERROR;
			}
			header->SequenceNumber++;
		} // ----------------------------------------------------------------------------------------------

		// waiting for connect response - DATA SYN ACK ---------------------------------------------------
		{
			cond.flag = (MACFrame_Flags_DATA | MACFrame_Flags_SYN | MACFrame_Flags_ACK);
			cond.data = 0; // not data-message
			cond.headerSrc = neighboringNodes[0]; // from next node
			cond.headerDst = header->SourceID; // to this node
			cond.routingSrc = extremeNodes[1]; // from end node
			cond.routingDst = extremeNodes[0]; // to start node
			result = _Routing_WaitResponse(&cond, 0);
			if ( result != Routing_SUCCESS ) {
				Debugger_SendStr("\3\2");
				_Routing_PopFromTables( extremeNodes[1] );
				return result;
			}
		} // ----------------------------------------------------------------------------------------------

		// send connect ack - DATA SYN ACK ---------------------------------------------------------------
		{
			header->DestinationID = neighboringNodes[1]; // to previous node
			header->Flags = (MACFrame_Flags_DATA | MACFrame_Flags_SYN | MACFrame_Flags_ACK);

			tx_config.tx_buffer_size = _Routing_FillCommonBuffer(header, rev_extremeNodes, 2);
			if ( Transceiver_Transmit( &tx_config ) != Transceiver_TXFRS ) {
				return Routing_ERROR;
			}
			header->SequenceNumber++;
		} // ----------------------------------------------------------------------------------------------

		//for (int i = 0, j = 0; i < _Routing_transactionSize; ++i, ++j)
		{
			// waiting for data - DATA --------------------------------------------------------------
			{
				cond.flag = (MACFrame_Flags_DATA);
				cond.data = 1; // data-message
				cond.headerSrc = neighboringNodes[1]; // from previous node
				cond.headerDst = header->SourceID; // to this node
				cond.routingSrc = 0; // data massege - without routing src
				cond.routingDst = 0; // data massege - without routing dst
				result = _Routing_WaitResponse(&cond, &respSize);
				if ( result != Routing_SUCCESS ) {
					Debugger_SendStr("\3\3");
					_Routing_PopFromTables( extremeNodes[1] );
					return result;
				}
			} // ------------------------------------------------------------------------------------------

			// send data - TOKEN DATA ---------------------------------------------------------------------
			{
				header->DestinationID = neighboringNodes[0]; // to next node
				header->Flags = (MACFrame_Flags_DATA);

				tx_config.tx_buffer_size = _Routing_FillCommonBuffer(
						header,
						&(_Routing_commonBuffer[MACFrame_PAYLOAD_OFFSET]),
						respSize - MACFrame_HEADER_SIZE - MACFrame_FCS_SIZE);
				if ( Transceiver_Transmit( &tx_config ) != Transceiver_TXFRS ) {
					return Routing_ERROR;
				}
				header->SequenceNumber++;
			} // ------------------------------------------------------------------------------------------

			//if ( i == (_Routing_transactionSize - 1) || (j % _Routing_trustPacks) == 0 )
			{
				// waiting for data ack - DATA ACK --------------------------------------------------
				{
					cond.flag = (MACFrame_Flags_DATA | MACFrame_Flags_ACK);
					cond.data = 0; // not data-message
					cond.headerSrc = neighboringNodes[0]; // from next node
					cond.headerDst = header->SourceID; // to this node
					cond.routingSrc = extremeNodes[1]; // from end node
					cond.routingDst = extremeNodes[0]; // to start node
					result = _Routing_WaitResponse(&cond, 0);
					if ( result != Routing_SUCCESS ) {
						_Routing_PopFromTables( extremeNodes[1] );
						Debugger_SendStr("\3\4");
						return result;
					}
				} // --------------------------------------------------------------------------------------

				// send data ack - DATA ACK ---------------------------------------------------------
				{
					header->DestinationID = neighboringNodes[1]; // to previous node
					header->Flags = (MACFrame_Flags_DATA | MACFrame_Flags_ACK);

					tx_config.tx_buffer_size = _Routing_FillCommonBuffer(header, rev_extremeNodes, 2);
					if ( Transceiver_Transmit( &tx_config ) != Transceiver_TXFRS ) {
						return Routing_ERROR;
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
			header->Flags = (MACFrame_Flags_DATA | MACFrame_Flags_SYN | MACFrame_Flags_ACK);

			tx_config.tx_buffer_size = _Routing_FillCommonBuffer(header, rev_extremeNodes, 2);
			if ( Transceiver_Transmit( &tx_config ) != Transceiver_TXFRS )  {
				return Routing_ERROR;
			}
		} // ----------------------------------------------------------------------------------------------

		//for (int i = 0, j = 0; i < _Routing_transactionSize; ++i, ++j)
		{
			// waiting for data - TOKEN DATA --------------------------------------------------------------
			{
				cond.flag = (MACFrame_Flags_DATA);
				cond.data = 1; // data-message
				cond.headerSrc = neighboringNodes[1]; // from previous node
				cond.headerDst = header->SourceID; // to this node
				cond.routingSrc = 0; // data massege - without routing src
				cond.routingDst = 0; // data massege - without routing dst
				result = _Routing_WaitResponse(&cond, data_buffer_size);
				if ( result != Routing_SUCCESS ) {
					Debugger_SendStr("\3\6");
					return result;
				}
				*data_buffer_size = (*data_buffer_size > tmp_data_buffer_size)? tmp_data_buffer_size : *data_buffer_size;
				_Routing_commonBuffer[MACFrame_SOURCE_ADDRESS_OFFSET] = extremeNodes[0];
				memcpy(data_buffer, _Routing_commonBuffer, *data_buffer_size);
			} // ------------------------------------------------------------------------------------------

			//if ( i == (_Routing_transactionSize - 1) || (j % _Routing_trustPacks) == 0 )
			{
				// send data ACK - TOKEN DATA ACK ---------------------------------------------------------
				{
					header->DestinationID = neighboringNodes[1]; // to previous node
					header->Flags = (MACFrame_Flags_DATA | MACFrame_Flags_ACK);

					tx_config.tx_buffer_size = _Routing_FillCommonBuffer(header, rev_extremeNodes, 2);
					if ( Transceiver_Transmit( &tx_config ) != Transceiver_TXFRS ) {
						return Routing_ERROR;
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
	for (uint8_t i = 0; i < Routing_TABLE_SIZE; ++i) {
		if (_Routing_newTable.hops[i] != _Routing_WORST_LVL) {
			_Routing_rxtxTable.units[i].owner = _Routing_newTable.ids[i];
			_Routing_rxtxTable.units[i].hop = _Routing_newTable.hops[i];
		} else {
			_Routing_rxtxTable.units[i].owner = _Routing_oldTable.ids[i];
			_Routing_rxtxTable.units[i].hop = _Routing_oldTable.hops[i];
		}
	}

	uint8_t size = _Routing_ExternalTableToBuffer( &_Routing_rxtxTable, _Routing_commonBuffer );
	TokenExt_RESULT TERes = TokenExt_Transfer( header, _Routing_commonBuffer, size );

	if (TERes == TokenExt_NEWCYCLE)
		_Routing_RefreshInternalTables();
}



void Routing_RecvTokenWithTable(MACHeader_Typedef *header, const uint8_t *rx_buffer)
{
	uint8_t rxlvl = Transceiver_GetLevelOfLastReceived();
	TokenExt_Receipt( header, rx_buffer );

	if ( _Routing_BufferToTable( &_Routing_rxtxTable, &(rx_buffer[TokenExt_PAYLOAD_OFFSET])) >= 0 ) {
		_Routing_Refresh_newTable( &_Routing_rxtxTable, rx_buffer[MACFrame_SOURCE_ADDRESS_OFFSET], rxlvl );
	}

	if (TokenExt_isCaptured() == TokenExt_TRUE) {
		_Routing_RefreshInternalTables();
	}
}



void Routing_GetReturnedToken(const uint8_t *rx_buffer)
{
	TokenExt_GetReturnedToken(rx_buffer);
}



void Routing_GenerateToken(MACHeader_Typedef *header)
{
	if (TokenExt_Generate(header) == TokenExt_SUCCESS)
		_Routing_RefreshInternalTables();
}



void Routing_Initialization(const Routing_InitializationStruct *initializationStruct)
{
	for (uint8_t i = 0; i < Routing_TABLE_SIZE; ++i) {
		_Routing_oldTable.hops[i] = _Routing_WORST_LVL;
		_Routing_oldTable.ids[i] = _Routing_WORST_LVL;
		_Routing_newTable.hops[i] = _Routing_WORST_LVL;
		_Routing_newTable.ids[i] = _Routing_WORST_LVL;
	}

	_Routing_deviceID = initializationStruct->deviceID - 1; // simplifies the logic
	_Routing_ACKReceivingTimeOut = initializationStruct->ACKReceivingTimeOut;
	_Routing_THRESHHOLD_LVL = initializationStruct->MinSignalLevel;
	EventTimer_SetPrescaler(Routing_TIM_16bit, SystemCoreClock / 1000); // ms

	tx_config.tx_buffer = _Routing_commonBuffer;
	tx_config.tx_delay = 0;
	tx_config.ranging = 0;
	tx_config.rx_aftertx_delay = 0;
	tx_config.rx_timeout = 0;
	tx_config.rx_buffer = 0;
	tx_config.rx_buffer_size = 0;
}



static inline uint8_t _Routing_DefineNewHops(uint8_t lvl)
{
	return lvl + 1;
}



static void _Routing_Refresh_newTable(const _Routing_ExternalTable *table, uint8_t tableOwnerID, uint8_t tableOwnerRxlvl)
{
	Debugger_ConstrAndSendBuf(3, 2, tableOwnerID, tableOwnerRxlvl);

	if (tableOwnerRxlvl > _Routing_THRESHHOLD_LVL) {
		tableOwnerID--;
		for (uint8_t i = 0, tmp; i < Routing_TABLE_SIZE; ++i) { // 'i' defines id of device in positional table
			if (table->units[i]._raw != _Routing_WORST_LVL &&
				table->units[i].owner != _Routing_deviceID &&
				i != _Routing_deviceID
			) {
				tmp = _Routing_DefineNewHops( table->units[i].hop );
				if (_Routing_newTable.hops[i] >= tmp) {
					_Routing_newTable.hops[i] = tmp;
					_Routing_newTable.ids[i] = tableOwnerID;
				}
			}
		}
		_Routing_newTable.ids[tableOwnerID] = tableOwnerID;
		_Routing_newTable.hops[tableOwnerID] = 1;
	}

	//tmp = _Routing_DefineNewLevels(((double)buffer[i] / 100.0) * ((double)tableOwnerRxlvl / 100.0)) * 100.0;
}



static void _Routing_RefreshInternalTables()
{
	Debugger_PushSymBack(1);
	Debugger_PushArrayBack(_Routing_oldTable.hops, Routing_TABLE_SIZE);
	Debugger_PushArrayBack(_Routing_oldTable.ids, Routing_TABLE_SIZE);
	Debugger_SendPreparedBuf();

	for (uint8_t i = 0; i < Routing_TABLE_SIZE; ++i) {
		_Routing_oldTable.hops[i] = _Routing_newTable.hops[i];
		_Routing_oldTable.ids[i] = _Routing_newTable.ids[i];

		_Routing_newTable.hops[i] = _Routing_WORST_LVL;
		_Routing_newTable.ids[i] = _Routing_WORST_LVL;
	}
}



static uint8_t _Routing_GetIntermediateID(uint8_t destinationID)
{
	destinationID--;
	if (_Routing_newTable.hops[destinationID] != _Routing_WORST_LVL)
		return _Routing_newTable.ids[destinationID] + 1;
	if (_Routing_oldTable.hops[destinationID] != _Routing_WORST_LVL)
		return _Routing_oldTable.ids[destinationID] + 1;
	return _Routing_WORST_LVL;
}



static Routing_RESULT _Routing_WaitResponse(_Routing_WaitResponseConditions *cond, uint8_t *respSize)
{
	Transceiver_RESULT trRes;
	uint8_t receivingState = 0;
	EventTimer_Reset(Routing_TIM_16bit);
	EventTimer_SetPeriod(Routing_TIM_16bit, _Routing_ACKReceivingTimeOut);
	EventTimer_Enable(Routing_TIM_16bit);
	while (1) {
		if (EventTimer_GetState(Routing_TIM_16bit) == EventTimer_SET) {
			EventTimer_Disable(Routing_TIM_16bit);
			return Routing_ERROR;
		}

		switch (receivingState) {
			case 0: // receiver turn on
			{
				Transceiver_ReceiverOn();
				receivingState = 1; // waiting request
			} break;
			case 1: // get data if it is available
			{
				trRes = Transceiver_GetReceptionResult();
				if (trRes == Transceiver_RXFCG) {
					if (respSize != 0)
						*respSize = Transceiver_GetAvailableData(_Routing_commonBuffer);
					else
						Transceiver_GetAvailableData(_Routing_commonBuffer);
					if (
						_Routing_commonBuffer[MACFrame_FLAGS_OFFSET] == cond->flag &&
						_Routing_commonBuffer[MACFrame_SOURCE_ADDRESS_OFFSET] == cond->headerSrc &&
						_Routing_commonBuffer[MACFrame_DESTINATION_ADDRESS_OFFSET] == cond->headerDst &&
						(cond->data ||
						(_Routing_commonBuffer[_Routing_SOURCE_OFFSET] == cond->routingSrc &&
						_Routing_commonBuffer[_Routing_DESTINATION_OFFSET] == cond->routingDst))
					) {
						EventTimer_Disable(Routing_TIM_16bit);
						return Routing_SUCCESS;
					} else {
						receivingState = 0; // reset
					}
				} else if (trRes) {
					return Routing_INTERRUPT; // todo
				}
			} break;
		}
	}
}



static int _Routing_BufferToTable(_Routing_ExternalTable *table, const uint8_t *buffer)
{
	for (uint8_t i = 0; i < Routing_TABLE_SIZE; ++i) {
		table->units[i]._raw = buffer[i];
	}
	return Routing_TABLE_SIZE;
}



static int _Routing_ExternalTableToBuffer(const _Routing_ExternalTable *table, uint8_t *buffer)
{
	for (uint8_t i = 0; i < Routing_TABLE_SIZE; ++i) {
		buffer[i] = table->units[i]._raw;
	}
	return Routing_TABLE_SIZE;
}



static uint8_t _Routing_FillCommonBuffer(
	const MACHeader_Typedef *header,
	const uint8_t *payload,
	uint8_t payload_size)
{
	uint8_t i = 0;
	// header
	_Routing_commonBuffer[i++] = (uint8_t)(header->FrameControl);
	_Routing_commonBuffer[i++] = (uint8_t)(header->FrameControl >> 8);
	_Routing_commonBuffer[i++] = (uint8_t)(header->SequenceNumber);
	_Routing_commonBuffer[i++] = (uint8_t)(header->PAN_ID);
	_Routing_commonBuffer[i++] = (uint8_t)(header->PAN_ID >> 8);
	_Routing_commonBuffer[i++] = (uint8_t)(header->DestinationID);
	_Routing_commonBuffer[i++] = (uint8_t)(header->DestinationID >> 8);
	_Routing_commonBuffer[i++] = (uint8_t)(header->SourceID);
	_Routing_commonBuffer[i++] = (uint8_t)(header->SourceID >> 8);
	_Routing_commonBuffer[i++] = (uint8_t)(header->Flags);
	// payload
	for (uint8_t j = 0; j < payload_size; ++j) {
		_Routing_commonBuffer[i++] = payload[j];
	}
	// FCS
	_Routing_commonBuffer[i++] = 0;
	_Routing_commonBuffer[i++] = 0;
	return i;
}



static void _Routing_PopFromTables(uint8_t deviceID)
{
	deviceID--;
	_Routing_oldTable.hops[deviceID] = _Routing_WORST_LVL;
	_Routing_oldTable.ids[deviceID] = _Routing_WORST_LVL;
	_Routing_newTable.hops[deviceID] = _Routing_WORST_LVL;
	_Routing_newTable.ids[deviceID] = _Routing_WORST_LVL;
}




