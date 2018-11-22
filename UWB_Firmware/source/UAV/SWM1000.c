
#include "USARTHandler.h"
#include "SWM1000.h"
#include "Random.h"
#include "Ranging.h"
#include "mUSART_DMA.h"
#include <string.h>
#include <stdio.h>



#define _SWM1000_FRAME_CONTROL			0x8841		// ckeck IEEE Std 802.15.4-2011 for details
#define _SWM1000_FRAME_FILTER_MASK		(DWT_FF_DATA_EN) // ckeck IEEE Std 802.15.4-2011 for details
#define _Poll_Timer 					TIM2



static MACHeader_Typedef _Main_MACHeader;
static UserPack _Distance_UserPack;
static uint16_t InitiatorID, prevInitiatorID;
static uint8_t isInitiatorDenied;
static uint16_t timeslot;



static uint8 UWBbuffer[MACFrame_FRAME_MAX_SIZE];



static void _SWM1000_Receiving(const Transceiver_RxConfig *rx_config);
static void _SWM1000_SendDistance(uint16_t destID);
static void _SWM1000_SetDefaultDistancePack(void);
static void _SWM1000_SetupTimerNewTimeSlot(void);
static uint8_t _SWM1000_SendConnectReqFrame(void);
static void _SWM1000_ConnectWithInitiator(Transceiver_RxConfig *rx_config);
static void _SWM1000_CpyMainHeaderToBuffer(uint8_t *buf);



void SWM1000_Loop(void)
{
	int MainState;
	Transceiver_RxConfig rx_config;

	MainState = 1;
	rx_config.rx_buffer = UWBbuffer;
	rx_config.rx_buffer_size = MACFrame_FRAME_MAX_SIZE;
	_SWM1000_SetupTimerNewTimeSlot(); // first slot

	int loopCnt = 0;

	while (1)
	{
		IWDG_ReloadCounter();

		// receive data from HL
		loopCnt++;
		if (loopCnt > 7) {
			loopCnt = 0;
			USARTHandler_Receive(&_Distance_UserPack);
		}

		switch (MainState)
		{
			// receiver on
			case 1:
			{
				Transceiver_ReceiverOn();
				MainState = 2;
			} break;
			// wait timeout or frame
			case 2:
			{
				if ( Transceiver_GetReceptionResult() == Transceiver_RXFCG ) {
					rx_config.rx_buffer_size = Transceiver_GetAvailableData(rx_config.rx_buffer);
					_SWM1000_Receiving( &rx_config );
					MainState = 1;
				}
				if (EventTimer_GetState(_Poll_Timer) == EventTimer_SET) {
					_SWM1000_SetupTimerNewTimeSlot();
					Transceiver_ReceiverOff();
					MainState = 3;
				}
			} break;
			// send connection request or connect with initiator
			case 3:
			{
				if (InitiatorID) {
					_SWM1000_ConnectWithInitiator(&rx_config);
				} else {
					_SWM1000_SendConnectReqFrame();
				}
				MainState = 1;
			} break;
		}
	}
}



static void _SWM1000_Receiving(const Transceiver_RxConfig *rx_config)
{
	uint16_t flag = rx_config->rx_buffer[MACFrame_FLAGS_OFFSET];

	switch (flag)
	{
		// rng init frame was received ***********************************************************************************
		case MACFrame_Flags_RNG:
		{
#ifndef SWM1000_FILTERING
			{ // check destination ID
				uint16_t destID = (rx_config->rx_buffer[MACFrame_DESTINATION_ADDRESS_OFFSET] |
								(rx_config->rx_buffer[MACFrame_DESTINATION_ADDRESS_OFFSET+1] << 8));
				if (destID != _Distance_UserPack.id) {
					_SWM1000_SetupTimerNewTimeSlot();
					return;
				}
			}
#endif
			uint16_t distance;

			_Main_MACHeader.SourceID = _Distance_UserPack.id;
			_Main_MACHeader.DestinationID = (uint16_t)
					(rx_config->rx_buffer[MACFrame_SOURCE_ADDRESS_OFFSET] |
					(rx_config->rx_buffer[MACFrame_SOURCE_ADDRESS_OFFSET+1] << 8)); // low byte first

			Ranging_RESULT RngRes = Ranging_GetDistance( &_Main_MACHeader, _Distance_UserPack.payload, &distance );
			if (RngRes == Ranging_SUCCESS) {
#ifndef SWM1000_ASCII_SYM
				UserPack upack;
				upack.id = _Main_MACHeader.DestinationID;
				upack.distance = distance;
				memcpy(upack.payload, rx_config->rx_buffer+MACFrame_PAYLOAD_OFFSET, UserPack_PAYLOAD_SIZE);
				USARTHandler_Send(&upack);
#else
				uint8_t buffer[128];
				char tmp[UserPack_PAYLOAD_SIZE+1];
				memcpy(tmp, rx_config->rx_buffer+MACFrame_PAYLOAD_OFFSET, UserPack_PAYLOAD_SIZE);
				tmp[UserPack_PAYLOAD_SIZE] = 0;
				uint8_t it = sprintf(buffer, "{%d}-{%d}-{%s}-{%d}\r\n",
						_Main_MACHeader.SourceID,
						_Main_MACHeader.DestinationID,
						tmp,
						distance);
				USART_SendBuffer(buffer, it);
#endif
			}

		} break;
		// CTS was received **********************************************************************************************
		case MACFrame_Flags_RST:
		{
			_SWM1000_SetupTimerNewTimeSlot();
		} break;
		// connect was established ***************************************************************************************
		case MACFrame_Flags_ACK:
		{
#ifndef SWM1000_FILTERING
			{ // check destination ID
				uint16_t destID = (rx_config->rx_buffer[MACFrame_DESTINATION_ADDRESS_OFFSET] |
								(rx_config->rx_buffer[MACFrame_DESTINATION_ADDRESS_OFFSET+1] << 8));
				if (destID != _Distance_UserPack.id) {
					_SWM1000_SetupTimerNewTimeSlot();
					return;
				}
			}
#endif
#ifdef SWM1000_ASCII_SYM
			sprintf(_Distance_UserPack.payload, "%.3d", timeslot);
#endif
			uint16_t destID = (uint16_t)
					(rx_config->rx_buffer[MACFrame_SOURCE_ADDRESS_OFFSET] |
					(rx_config->rx_buffer[MACFrame_SOURCE_ADDRESS_OFFSET+1] << 8));

			_SWM1000_SendDistance(destID);
		} break;
		// needs to connect with initiator *******************************************************************************
		case MACFrame_Flags_SYN:
		{
			InitiatorID = (uint16_t)
					(rx_config->rx_buffer[MACFrame_SOURCE_ADDRESS_OFFSET] |
					(rx_config->rx_buffer[MACFrame_SOURCE_ADDRESS_OFFSET+1] << 8));
		} break;
	}
}



static void _SWM1000_SendDistance(uint16_t destID)
{
	Transceiver_RESULT tr_res;
	Transceiver_TxConfig tx_config;
	uint16_t distance;
	Ranging_RESULT RngRes;
	uint8_t rxbuf[UserPack_PACK_SIZE];
	uint8_t buffer[ sizeof(MACHeader_Typedef) + 2 ];

	dwt_setaddress16(_Distance_UserPack.id);
	_Main_MACHeader.SourceID = _Distance_UserPack.id;
	_Main_MACHeader.DestinationID = 0xFFFF;
	_Main_MACHeader.Flags = MACFrame_Flags_RST;

	// prepare CTS
	_SWM1000_CpyMainHeaderToBuffer(buffer);

	tx_config.tx_buffer = buffer;
	tx_config.tx_buffer_size = sizeof(buffer);
	tx_config.tx_delay = 0;
	tx_config.ranging = 0;
	tx_config.rx_aftertx_delay = 0;
	tx_config.rx_timeout = 0;
	tx_config.rx_buffer = 0;
	tx_config.rx_buffer_size = 0;

	// send CTS
	tr_res = Transceiver_Transmit( &tx_config );

	if (tr_res == Transceiver_TXFRS) {
		// initiate rng
		_Main_MACHeader.DestinationID = destID;
		_Main_MACHeader.Flags = MACFrame_Flags_NOP;

		RngRes = Ranging_Initiate(&_Main_MACHeader,_Distance_UserPack.payload, rxbuf, &distance);
		if (RngRes == Ranging_SUCCESS) {
#ifndef SWM1000_ASCII_SYM
			UserPack upack;
			upack.id = _Main_MACHeader.DestinationID;
			upack.distance = distance;
			memcpy(upack.payload, rxbuf, UserPack_PAYLOAD_SIZE);
			USARTHandler_Send(&upack);
#else
			uint8_t buffer[128];
			char tmp[UserPack_PAYLOAD_SIZE+1];
			memcpy(tmp, rxbuf, UserPack_PAYLOAD_SIZE);
			tmp[UserPack_PAYLOAD_SIZE] = 0;
			uint8_t it = sprintf(buffer, "{%d}-{%d}-{%s}-{%d}\r\n",
					_Main_MACHeader.SourceID,
					_Main_MACHeader.DestinationID,
					tmp,
					distance);
			USART_SendBuffer(buffer, it);
#endif
		}
	}
}



static uint8_t _SWM1000_SendConnectReqFrame()
{
	Transceiver_RESULT tr_res;
	Transceiver_TxConfig tx_config;
	uint8_t buffer[ sizeof(MACHeader_Typedef) + 2 ];

	dwt_setaddress16(_Distance_UserPack.id);
	_Main_MACHeader.SourceID = _Distance_UserPack.id;
	_Main_MACHeader.DestinationID = 0xFFFF;
	_Main_MACHeader.Flags = MACFrame_Flags_SYN;

	// prepare req frame
	_SWM1000_CpyMainHeaderToBuffer(buffer);

	tx_config.tx_buffer = buffer;
	tx_config.tx_buffer_size = sizeof(buffer);
	tx_config.tx_delay = 0;
	tx_config.ranging = 0;
	tx_config.rx_aftertx_delay = 0;
	tx_config.rx_timeout = 0;
	tx_config.rx_buffer = 0;
	tx_config.rx_buffer_size = 0;

	tr_res = Transceiver_Transmit( &tx_config );

	return tr_res == Transceiver_TXFRS;
}



static void _SWM1000_ConnectWithInitiator(Transceiver_RxConfig *rx_config)
{
	// without 2 connection with the same initiator in a row
	if (prevInitiatorID == InitiatorID && isInitiatorDenied) {
		isInitiatorDenied = 0; // next - isn't denied
		return;
	}

	Transceiver_RESULT tr_res;
	Transceiver_TxConfig tx_config;
	uint8_t buffer[ sizeof(MACHeader_Typedef) + 2 ];

	dwt_setaddress16(_Distance_UserPack.id);
	_Main_MACHeader.SourceID = _Distance_UserPack.id;
	_Main_MACHeader.DestinationID = InitiatorID;
	_Main_MACHeader.Flags = MACFrame_Flags_ACK;

	// prepare ans frame
	_SWM1000_CpyMainHeaderToBuffer(buffer);

	tx_config.tx_buffer = buffer;
	tx_config.tx_buffer_size = sizeof(buffer);
	tx_config.tx_delay = 0;
	tx_config.ranging = 0;
	tx_config.rx_aftertx_delay = 100;
	tx_config.rx_timeout = 1500;
	tx_config.rx_buffer = rx_config->rx_buffer;
	tx_config.rx_buffer_size = rx_config->rx_buffer_size;

	tr_res = Transceiver_Transmit( &tx_config );

	if (tr_res == Transceiver_RXFCG) {
		_SWM1000_Receiving(rx_config);
	}

	prevInitiatorID = InitiatorID;
	isInitiatorDenied = 1;
	// reset
	InitiatorID = 0;
}



void SWM1000_Initialization()
{
	peripherals_init();

	// deca init
	Transceiver_Initialization();

	// header init
	_Main_MACHeader.FrameControl = _SWM1000_FRAME_CONTROL;
	_Main_MACHeader.SequenceNumber = 0;
	_Main_MACHeader.PAN_ID = SWM1000_PANID;
	_Main_MACHeader.SourceID = 0;

	// Initialises network params
	dwt_setpanid(SWM1000_PANID);
	dwt_setaddress16(0);
#ifdef SWM1000_FILTERING
	dwt_enableframefilter(_SWM1000_FRAME_FILTER_MASK);
#endif

	// modules init
	USARTHandler_Initialization();
	Random_Initialization();
	Ranging_Initialization();
	_SWM1000_SetDefaultDistancePack();

	// timer for polling
	EventTimer_SetPrescaler(_Poll_Timer, SystemCoreClock / 1000); // ms

	// watchdog settings - once at the power on
	IWDG_SetPrescaler(IWDG_Prescaler_32); // 1.25 kHz
	IWDG_SetReload(10 & 0x0FFF);
	IWDG_ReloadCounter();
#ifndef DEBUG
	IWDG_Enable();
#endif
}



static void _SWM1000_CpyMainHeaderToBuffer(uint8_t *buf)
{
	uint8_t i = 0;
	buf[i++] = (uint8)(_Main_MACHeader.FrameControl);
	buf[i++] = (uint8)(_Main_MACHeader.FrameControl >> 8);
	buf[i++] = (uint8)(_Main_MACHeader.SequenceNumber);
	buf[i++] = (uint8)(_Main_MACHeader.PAN_ID);
	buf[i++] = (uint8)(_Main_MACHeader.PAN_ID >> 8);
	buf[i++] = (uint8)(_Main_MACHeader.DestinationID);
	buf[i++] = (uint8)(_Main_MACHeader.DestinationID >> 8);
	buf[i++] = (uint8)(_Main_MACHeader.SourceID);
	buf[i++] = (uint8)(_Main_MACHeader.SourceID >> 8);
	buf[i++] = (uint8)(_Main_MACHeader.Flags);
}



static void _SWM1000_SetDefaultDistancePack()
{
	_Distance_UserPack.id = Random_GetNumber(10, SWM1000_PAN_SIZE);
	memset(_Distance_UserPack.payload, 0, UserPack_PAYLOAD_SIZE);
	_Distance_UserPack.distance = 0;
}



static void _SWM1000_SetupTimerNewTimeSlot()
{
	EventTimer_Disable(_Poll_Timer);
	EventTimer_Reset(_Poll_Timer);
	timeslot = Random_GetNumber(3, SWM1000_TIMESLOT_MAX_DUR) * SWM1000_TIMESLOT_DURATION;
	EventTimer_SetPeriod(_Poll_Timer, timeslot);
	EventTimer_Enable(_Poll_Timer);
}


