
#include <string.h>
#include "Ranging.h"


/*
	Three types of messages: 
		init 	- Initiator -> Responder, Responder {!cmp} ->
		resp 	- Responder -> Initiator, Initiator {cmp} ->
		final 	- Initiator -> Responder, Responder {cmp} -> return

*/


// Default antenna delay values for 64 MHz PRF. See NOTE 1 below.
#define TX_ANT_DELAY 16476 //16436
#define RX_ANT_DELAY 16476 //16436

// Speed of light in air, in metres per second.
#define SPEED_OF_LIGHT 299702547

// UWB microsecond (uus) to device time unit (dtu, around 15.65 ps) conversion factor.
// 1 uus = 512 / 499.2 µs and 1 µs = 499.2 * 128 dtu.
#define UUS_TO_DWT_TIME 65536


// init->resp:  rx after tx
#define INIT_TX_TO_RESP_RX_DELAY_UUS 		150 	//150
// init->resp:  rx timeout
#define RESP_RX_TIMEOUT_UUS 				1500 	//2700
// resp->init:  tx delay
#define INIT_RX_TO_RESP_TX_DELAY_UUS 		1300 // 570	 //1100 	//2600		(before resp)  <---
// resp->init:  rx after tx
#define RESP_TX_TO_FINAL_RX_DELAY_UUS		250 	//500
// init->resp:  tx delay
#define RESP_RX_TO_FINAL_TX_DELAY_UUS 		1600 // 600	 //1400 	//3100		(before final) <---
// resp->init:  rx timeout
#define FINAL_RX_TIMEOUT_UUS 				1800 	//3300

// Timestamps of frames transmission/reception.
// As they are 40-bit wide, we need to define a 64-bit int type to handle them.
typedef unsigned long long uint64;
typedef signed long long int64;

#define RESP_IDENTIFIER_SIZE			2
static const uint8 resp_identifier[RESP_IDENTIFIER_SIZE] = {0xAA, 0x55};

#define FINAL_IDENTIFIER_SIZE			2
static const uint8 final_identifier[FINAL_IDENTIFIER_SIZE] = {0x55, 0xAA};

// Indexes to access some of the fields in the frames defined above.
#define FINAL_MSG_TS_LEN 				4
#define FINAL_MSG_INIT_TS_OFFSET 		(MACFrame_HEADER_SIZE + FINAL_IDENTIFIER_SIZE)
#define FINAL_MSG_RESP_TS_OFFSET 		(FINAL_MSG_INIT_TS_OFFSET + FINAL_MSG_TS_LEN)
#define FINAL_MSG_FINAL_TS_OFFSET 		(FINAL_MSG_RESP_TS_OFFSET + FINAL_MSG_TS_LEN)
#define FINAL_MSG_ALL_TS_SIZE			(FINAL_MSG_TS_LEN * 3)

// Declaration of static functions.
static uint64 _Ranging_GetTxTs64(void);
static uint64 _Ranging_GetRxTs64(void);
static void _Ranging_FinalMsgGetTs(const uint8 *ts_field, uint32 *ts);
static inline uint16_t _Ranging_ConvertDistanceToCm(double distance);

static void _Ranging_SetInitMsg(
	MACHeader_Typedef *header,
	uint8 *msg, 
	const uint8 *payload
);
static void _Ranging_SetRespMsg(
	MACHeader_Typedef *header,
	uint8 *msg
);
static void _Ranging_SetFinalMsg(
	MACHeader_Typedef *header,
	uint8 *msg, 
	uint64 init_ts, 
	uint64 resp_ts, 
	uint64 final_ts
);



uint8 init_msg[ MACFrame_HEADER_SIZE + Ranging_PAYLOAD_SIZE + MACFrame_FCS_SIZE ]; // 18
uint8 resp_msg[ MACFrame_HEADER_SIZE + RESP_IDENTIFIER_SIZE + MACFrame_FCS_SIZE ]; // 14
uint8 expected_resp_msg[ sizeof(resp_msg) ];
uint8 final_msg[ MACFrame_HEADER_SIZE + FINAL_IDENTIFIER_SIZE + FINAL_MSG_ALL_TS_SIZE + MACFrame_FCS_SIZE ]; // 26
uint8 expected_final_msg[ sizeof(final_msg) ];



Ranging_RESULT Ranging_Initiate(MACHeader_Typedef *header, const uint8_t *payload)
{
	Transceiver_RESULT tr_res;
	Transceiver_TxConfig tx_config;		
	
	header->Flags = MACFrame_Flags_RNG;
	
	_Ranging_SetInitMsg(header, init_msg, payload); 
	
	tx_config.tx_buffer = init_msg;
	tx_config.tx_buffer_size = sizeof(init_msg);
	tx_config.tx_delay = 0;
	tx_config.ranging = 1;
	tx_config.rx_aftertx_delay = INIT_TX_TO_RESP_RX_DELAY_UUS;
	tx_config.rx_timeout = RESP_RX_TIMEOUT_UUS;
	tx_config.rx_buffer = resp_msg;
	tx_config.rx_buffer_size = sizeof(resp_msg);	
	
	tr_res = Transceiver_Transmit( &tx_config );	
	
	if (tr_res == Transceiver_RXFCG) {	
		MACHeader_Typedef tmp_header;
		header->SequenceNumber++;
		
		tmp_header.FrameControl = header->FrameControl;
		tmp_header.SequenceNumber = tx_config.rx_buffer[MACFrame_SEQ_NUM_OFFSET];
		tmp_header.PAN_ID = header->PAN_ID;
		tmp_header.DestinationID = header->SourceID;
		tmp_header.SourceID = header->DestinationID;
		tmp_header.Flags = header->Flags;
		
		_Ranging_SetRespMsg( 
			&tmp_header,			
			expected_resp_msg
		);
		
		if ( memcmp(tx_config.rx_buffer, expected_resp_msg, sizeof(resp_msg) - MACFrame_FCS_SIZE) == 0 ) {
			// Time-stamps of frames transmission/reception, expressed in device time units.
			// As they are 40-bit wide, we need to define a 64-bit int type to handle them.
			uint64 init_tx_ts, resp_rx_ts, final_tx_ts;
			uint32 final_tx_time;

			// Retrieve poll transmission and response reception timestamp.
			init_tx_ts = _Ranging_GetTxTs64();
			resp_rx_ts = _Ranging_GetRxTs64();

			// Compute final message transmission time.
			final_tx_time = (resp_rx_ts + (RESP_RX_TO_FINAL_TX_DELAY_UUS * UUS_TO_DWT_TIME)) >> 8;
			
			// Final TX timestamp is the transmission time we programmed plus the TX antenna delay.
			final_tx_ts = (((uint64)(final_tx_time & 0xFFFFFFFEUL)) << 8) + TX_ANT_DELAY;
			
			_Ranging_SetFinalMsg( 
				header,
				final_msg, 
				init_tx_ts, 
				resp_rx_ts, 
				final_tx_ts 
			);
			
			tx_config.tx_buffer = final_msg;
			tx_config.tx_buffer_size = sizeof(final_msg);
			tx_config.tx_delay = final_tx_time;
			tx_config.rx_aftertx_delay = 0;			
			tx_config.rx_buffer = NULL;
			tx_config.rx_buffer_size = 0;
			tx_config.rx_timeout = 0;
			
			tr_res = Transceiver_Transmit( &tx_config );			
			
			if (tr_res == Transceiver_TXFRS) {	
				header->SequenceNumber++;
				return DWT_SUCCESS;
			}			
		}		
	}
	return DWT_ERROR;
}



Ranging_RESULT Ranging_GetDistance(MACHeader_Typedef *header, uint16_t *distance16)
{	
	uint64 init_rx_ts;
	uint32 resp_tx_time;	
	
	Transceiver_RESULT tr_res;
	Transceiver_TxConfig tx_config;
	
	header->Flags = MACFrame_Flags_RNG;

	// Retrieve poll reception timestamp.
	init_rx_ts = _Ranging_GetRxTs64();

	// Set send time for response. See NOTE 9 below.
	resp_tx_time = (init_rx_ts + (INIT_RX_TO_RESP_TX_DELAY_UUS * UUS_TO_DWT_TIME)) >> 8;
	
	_Ranging_SetRespMsg(header, expected_resp_msg);
	
	tx_config.tx_buffer = expected_resp_msg;
	tx_config.tx_buffer_size = sizeof(expected_resp_msg);
	tx_config.tx_delay = resp_tx_time;
	tx_config.ranging = 1;
	tx_config.rx_aftertx_delay = RESP_TX_TO_FINAL_RX_DELAY_UUS;
	tx_config.rx_buffer = final_msg;
	tx_config.rx_buffer_size = sizeof(final_msg);
	tx_config.rx_timeout = FINAL_RX_TIMEOUT_UUS;
	
	tr_res = Transceiver_Transmit( &tx_config );
	
	if (tr_res == Transceiver_RXFCG) {		
		MACHeader_Typedef tmp_header;
		header->SequenceNumber++;
		
		tmp_header.FrameControl = header->FrameControl;
		tmp_header.SequenceNumber = tx_config.rx_buffer[MACFrame_SEQ_NUM_OFFSET];
		tmp_header.PAN_ID = header->PAN_ID;
		tmp_header.DestinationID = header->SourceID;
		tmp_header.SourceID = header->DestinationID;
		tmp_header.Flags = header->Flags;
		
		_Ranging_SetFinalMsg(&tmp_header, expected_final_msg, 0, 0, 0);
		
		if ( 
			memcmp(
				tx_config.rx_buffer, 
				expected_final_msg, 
				sizeof(expected_final_msg) - (FINAL_MSG_ALL_TS_SIZE + MACFrame_FCS_SIZE)
			) == 0 
		) {
			uint32 init_tx_ts, resp_rx_ts, final_tx_ts;
			uint32 init_rx_ts_32, resp_tx_ts_32, final_rx_ts_32;
			uint64 resp_tx_ts, final_rx_ts;
			int64 tof_dtu;
			double Ra, Rb, Da, Db;
			double tof;		

			// Retrieve response transmission and final reception timestamps.
			resp_tx_ts = _Ranging_GetTxTs64();
			final_rx_ts = _Ranging_GetRxTs64();

			// Get timestamps embedded in the final message.
			_Ranging_FinalMsgGetTs(&tx_config.rx_buffer[FINAL_MSG_INIT_TS_OFFSET], &init_tx_ts);
			_Ranging_FinalMsgGetTs(&tx_config.rx_buffer[FINAL_MSG_RESP_TS_OFFSET], &resp_rx_ts);
			_Ranging_FinalMsgGetTs(&tx_config.rx_buffer[FINAL_MSG_FINAL_TS_OFFSET], &final_tx_ts);

			// Compute time of flight. 32-bit subtractions give correct answers even if clock has wrapped.
			init_rx_ts_32 = (uint32)init_rx_ts;
			resp_tx_ts_32 = (uint32)resp_tx_ts;
			final_rx_ts_32 = (uint32)final_rx_ts;
			Ra = (double)(resp_rx_ts - init_tx_ts);
			Rb = (double)(final_rx_ts_32 - resp_tx_ts_32);
			Da = (double)(final_tx_ts - resp_rx_ts);
			Db = (double)(resp_tx_ts_32 - init_rx_ts_32);
			tof_dtu = (int64)((Ra * Rb - Da * Db) / (Ra + Rb + Da + Db));

			tof = tof_dtu * DWT_TIME_UNITS;
			*distance16 = _Ranging_ConvertDistanceToCm(tof * SPEED_OF_LIGHT);
			return Ranging_SUCCESS;
		}
	}
	return Ranging_ERROR;
}



void Ranging_Initialization(void) 
{
	dwt_setrxantennadelay(RX_ANT_DELAY);
    dwt_settxantennadelay(TX_ANT_DELAY);
}



static void _Ranging_SetInitMsg(
	MACHeader_Typedef *header, 
	uint8 *msg, 
	const uint8 *payload
)
{
	uint8 i = 0;
	// header
	msg[i++] = (uint8)(header->FrameControl);
	msg[i++] = (uint8)(header->FrameControl >> 8);
	msg[i++] = (uint8)(header->SequenceNumber);
	msg[i++] = (uint8)(header->PAN_ID);
	msg[i++] = (uint8)(header->PAN_ID >> 8);
	msg[i++] = (uint8)(header->DestinationID);
	msg[i++] = (uint8)(header->DestinationID >> 8);
	msg[i++] = (uint8)(header->SourceID);
	msg[i++] = (uint8)(header->SourceID >> 8);
	msg[i++] = (uint8)(header->Flags);	
	// payload
	for (uint8 j = 0; j < RESP_IDENTIFIER_SIZE; ++j)
		msg[i++] = payload[j];
	// FCS
	msg[i++] = 0;
	msg[i++] = 0;
}



static void _Ranging_SetRespMsg(
	MACHeader_Typedef *header,
	uint8 *msg
)
{
	uint8 i = 0;
	// header
	msg[i++] = (uint8)(header->FrameControl);
	msg[i++] = (uint8)(header->FrameControl >> 8);
	msg[i++] = (uint8)(header->SequenceNumber);
	msg[i++] = (uint8)(header->PAN_ID);
	msg[i++] = (uint8)(header->PAN_ID >> 8);
	msg[i++] = (uint8)(header->DestinationID);
	msg[i++] = (uint8)(header->DestinationID >> 8);
	msg[i++] = (uint8)(header->SourceID);
	msg[i++] = (uint8)(header->SourceID >> 8);
	msg[i++] = (uint8)(header->Flags);	
	// payload
	for (uint8 j = 0; j < RESP_IDENTIFIER_SIZE; ++j)
		msg[i++] = resp_identifier[j];
	// FCS
	msg[i++] = 0;
	msg[i++] = 0;
}



static void _Ranging_SetFinalMsg(
	MACHeader_Typedef *header,
	uint8 *msg, 
	uint64 init_ts, 
	uint64 resp_ts, 
	uint64 final_ts
)
{
	uint8 i = 0;
	uint8 j;
	// header
	msg[i++] = (uint8)(header->FrameControl);
	msg[i++] = (uint8)(header->FrameControl >> 8);
	msg[i++] = (uint8)(header->SequenceNumber);
	msg[i++] = (uint8)(header->PAN_ID);
	msg[i++] = (uint8)(header->PAN_ID >> 8);
	msg[i++] = (uint8)(header->DestinationID);
	msg[i++] = (uint8)(header->DestinationID >> 8);
	msg[i++] = (uint8)(header->SourceID);
	msg[i++] = (uint8)(header->SourceID >> 8);
	msg[i++] = (uint8)(header->Flags);		
	// payload
	for (j = 0; j < FINAL_IDENTIFIER_SIZE; ++j) {
		msg[i++] = final_identifier[j];
	}
	for (j = 0; j < FINAL_MSG_TS_LEN; ++j) {
		msg[FINAL_MSG_INIT_TS_OFFSET + j] = (uint8)(init_ts >> (8 * j));
		msg[FINAL_MSG_RESP_TS_OFFSET + j] = (uint8)(resp_ts >> (8 * j));
		msg[FINAL_MSG_FINAL_TS_OFFSET + j] = (uint8)(final_ts >> (8 * j));
	}
	// FCS
	msg[MACFrame_HEADER_SIZE + FINAL_IDENTIFIER_SIZE + FINAL_MSG_ALL_TS_SIZE] = 0;
	msg[MACFrame_HEADER_SIZE + FINAL_IDENTIFIER_SIZE + FINAL_MSG_ALL_TS_SIZE + 1] = 0;
}



static inline uint16_t _Ranging_ConvertDistanceToCm(double distance)
{	
	return (uint16_t)(distance * 100);
}



static uint64 _Ranging_GetTxTs64(void)
{
    uint8 ts_tab[5];
    uint64 ts = 0;
    int i;
    dwt_readtxtimestamp(ts_tab);
    for (i = 4; i >= 0; --i) {
        ts <<= 8;
        ts |= ts_tab[i];
    }
    return ts;
}



static uint64 _Ranging_GetRxTs64(void)
{
    uint8 ts_tab[5];
    uint64 ts = 0;
    int i;
    dwt_readrxtimestamp(ts_tab);
    for (i = 4; i >= 0; --i) {
        ts <<= 8;
        ts |= ts_tab[i];
    }
    return ts;
}



static void _Ranging_FinalMsgGetTs(const uint8 *ts_field, uint32 *ts)
{
    int i;
    *ts = 0;
    for (i = 0; i < FINAL_MSG_TS_LEN; ++i) {
        *ts += ts_field[i] << (i * 8);
    }
}