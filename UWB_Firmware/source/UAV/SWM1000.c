
#include "USARTHandler.h"
#include "SWM1000.h"
#include "Routing.h"
#include "Ranging.h"
#include "Random.h"
#include "ConfigFW.h"
#include "Debugger.h"
#include <string.h>



typedef union {
	uint16 d;
	uint8 b[2];
} uint16_str;



typedef enum {
	_SWM1000_Nothing,
	_SWM1000_Generate,
	_SWM1000_Transmit,
	_SWM1000_Poll,
	_SWM1000_TokenTransfer
} _SWM1000_LoopState;



typedef enum {
	_SWM1000_TurnOn,
	_SWM1000_CheckData
} _SWM1000_ReceivingState;



#define _SWM1000_FRAME_CONTROL			0x8841		// ckeck IEEE Std 802.15.4-2011 for details
#define _SWM1000_FRAME_FILTER_MASK		(DWT_FF_DATA_EN) // ckeck IEEE Std 802.15.4-2011 for details



#define _Config_Timer 		TIM14
#define _ConfigTO_Timer		TIM2
#define _Generate_Timer 	TIM14
#define _Poll_Timer 		TIM2



static MACHeader_Typedef _Pointer_MACHeader;



static Ranging_RESULT RngRes;
static USARTHandler_RESULT UHRes;



static UserPack _Distance_UserPack;



static inline void _SWM1000_UserCommandHandler();
static void _SWM1000_ReceivingAutomat(Transceiver_RxConfig *rx_config);
static void _SWM1000_PutDistanceInUData(uint8 *payload, uint16_str distance);
static inline void _SWM1000_Configurate();
static inline void _SWM1000_SetDefaultConfiguration();
static UserPack_CommandRes _SWM1000_SendDistance();
static void _SWM1000_SetDefaultDistanceParams();
static void _SWM1000_SetDistanceParams(const UserPack *upack);



void SWM1000_Loop(void)
{
	uint8 buffer[MACFrame_FRAME_MAX_SIZE];
	_SWM1000_LoopState loopState;
	_SWM1000_ReceivingState recvState = _SWM1000_TurnOn;
	Transceiver_RxConfig rx_config;
	rx_config.rx_buffer = buffer;
	rx_config.rx_buffer_size = MACFrame_FRAME_MAX_SIZE;
	UserPack upack, retpack;
		
	GeneralTimer_Reset(_Generate_Timer);
	GeneralTimer_Enable(_Generate_Timer);

	while (1) 
	{
		IWDG_ReloadCounter();

		// receive data from HL
		if ( USARTHandler_isAvailableToReceive() == USARTHandler_TRUE )  {
			UHRes = USARTHandler_Receive(&upack);
			retpack.FCmd.res = UserPack_CmdRes_Accepted;
			retpack.SCmd.cmd = upack.FCmd.cmd;
			retpack.TotalSize = 0;
			USARTHandler_Send(&retpack);
		}	

		if (TokenExt_isCaptured() == TokenExt_TRUE) {
			if (UHRes == USARTHandler_SUCCESS)
				loopState = _SWM1000_Transmit;
			else if (GeneralTimer_GetState(_Poll_Timer) == GeneralTimer_SET)
				loopState = _SWM1000_Poll;
			else
				loopState = _SWM1000_TokenTransfer;
		} else if (GeneralTimer_GetState(_Generate_Timer) == GeneralTimer_SET) {
			loopState = _SWM1000_Generate;
		} else {
			loopState = _SWM1000_Nothing;
		}

		// ***************************************************************************************************
		// transmission **************************************************************************************
		if (loopState != _SWM1000_Nothing) {
			// reset receiver
			recvState = _SWM1000_TurnOn;
			Transceiver_ReceiverOff();	

			switch (loopState) {
				// generate token in the absence other requests
				case _SWM1000_Generate: {
					_Pointer_MACHeader.DestinationID = 0xFFFF;
					_Pointer_MACHeader.Flags = MACFrame_Flags_NOP;
					Routing_GenerateToken( &_Pointer_MACHeader );
					GeneralTimer_Reset(_Generate_Timer);
				} break;
				// if transfer is allowed
				case _SWM1000_Transmit: {
					UHRes = USARTHandler_ERROR; // just reset
					_SWM1000_UserCommandHandler(&upack);
				} break;
				// polling with distance transfer
				case _SWM1000_Poll: {
					_SWM1000_SetDefaultDistanceParams();
					// reset poll timer inside
					_SWM1000_SendDistance();
				} break;
				// if token is captured
				case _SWM1000_TokenTransfer: {
					_Pointer_MACHeader.DestinationID = 0xFFFF;
					_Pointer_MACHeader.Flags = MACFrame_Flags_NOP;
					Routing_SendTokenWithTable(&_Pointer_MACHeader);
				} break;
				default: {
					// nop
				} break;
			}
		}
		// ***************************************************************************************************
		// reception *****************************************************************************************
		else {
			switch (recvState) {
				case _SWM1000_TurnOn: // receiver turn on
				{					
					Transceiver_ReceiverOn();
					recvState = _SWM1000_CheckData; // waiting request
				} break;
				case _SWM1000_CheckData: // get data if it is available
				{
					if ( Transceiver_GetReceptionResult() == Transceiver_RXFCG ) {
						rx_config.rx_buffer_size = Transceiver_GetAvailableData(rx_config.rx_buffer); // read
						_SWM1000_ReceivingAutomat( &rx_config ); // respond
						recvState = _SWM1000_TurnOn; // reset
						GeneralTimer_Reset(_Generate_Timer);
					}
				} break;
			}
		}
		// ***************************************************************************************************
		// ***************************************************************************************************
	} // while (1)
}



static void _SWM1000_UserCommandHandler(UserPack *upack)
{
	UserPack_CommandRes result = UserPack_CmdRes_Fail;

	switch (upack->FCmd.cmd) {
		// ---------------------------------------------------------------------------------------------------
		case UserPack_Cmd_Service:
		{ // nop
		} break;
		// ---------------------------------------------------------------------------------------------------
		case UserPack_Cmd_SystemConfig:
		{
			result = UserPack_CmdRes_Success;
			_SWM1000_Configurate(upack);
		} break;
		// ---------------------------------------------------------------------------------------------------
		case UserPack_Cmd_Distance:
		{
			_SWM1000_SetDistanceParams(upack);
			result = _SWM1000_SendDistance();
		} break;
		// ---------------------------------------------------------------------------------------------------
		case UserPack_Cmd_Data:
		{
			_Pointer_MACHeader.DestinationID = upack->SCmd.devID & 0x00FF;
			_Pointer_MACHeader.Flags = MACFrame_Flags_NOP;
			Routing_RESULT RouRes = Routing_SendData(&_Pointer_MACHeader, upack->Data, upack->TotalSize);
			if ( RouRes == Routing_INTERRUPT ) {
				TokenExt_Reset();
			}
			result = (RouRes == Routing_SUCCESS)? UserPack_CmdRes_Success : UserPack_CmdRes_Fail;
		} break;
		// ---------------------------------------------------------------------------------------------------
		default:
		{ // nop
		} break;
		// ---------------------------------------------------------------------------------------------------
		
	} // switch (upack->Command)

	UserPack _upack;
	_upack.FCmd.res = result;
	_upack.SCmd.cmd = upack->FCmd.cmd;
	_upack.TotalSize = 0;
	USARTHandler_Send(&_upack);
}



static void _SWM1000_ReceivingAutomat(Transceiver_RxConfig *rx_config)
{
	switch ( rx_config->rx_buffer[MACFrame_FLAGS_OFFSET] ) {
		// ---------------------------------------------------------------------------------------------------
		// Ranging algorithm - getting distance
		case MACFrame_Flags_RNG:					
		{
			uint16_str distance;
			_Pointer_MACHeader.DestinationID = rx_config->rx_buffer[MACFrame_SOURCE_ADDRESS_OFFSET]; // low byte first	
			_Pointer_MACHeader.Flags = MACFrame_Flags_NOP;
			RngRes = Ranging_GetDistance( &_Pointer_MACHeader, &(distance.d) );
			if (RngRes == Ranging_SUCCESS) {
				UserPack upack;
				upack.FCmd.cmd = UserPack_Cmd_Distance;
				upack.SCmd.devID = rx_config->rx_buffer[MACFrame_SOURCE_ADDRESS_OFFSET];
				_SWM1000_PutDistanceInUData(&(rx_config->rx_buffer[MACFrame_PAYLOAD_OFFSET]), distance);
				upack.TotalSize = Ranging_PAYLOAD_SIZE + 2; // + 2 for distance
				UserPack_SetData(&upack, &(rx_config->rx_buffer[MACFrame_PAYLOAD_OFFSET]));
				USARTHandler_Send(&upack);
			}
		} break;						
		// ---------------------------------------------------------------------------------------------------
		// Try to catch token from another device
		case (MACFrame_Flags_TOKEN | MACFrame_Flags_SYN):
		{	
			_Pointer_MACHeader.DestinationID = MACFrame_BROADCAST_ID; // everybody should be able to hear this message
			_Pointer_MACHeader.Flags = MACFrame_Flags_NOP;
			Routing_RecvTokenWithTable(&_Pointer_MACHeader, rx_config->rx_buffer);
		} break;
		// ---------------------------------------------------------------------------------------------------
		// Data receiving algorithm - establish logical connection and receive / retranslate the data
		case (MACFrame_Flags_DATA | MACFrame_Flags_SYN):
		{
			_Pointer_MACHeader.DestinationID = rx_config->rx_buffer[MACFrame_SOURCE_ADDRESS_OFFSET];
			_Pointer_MACHeader.Flags = MACFrame_Flags_NOP;
			uint8_t data_buffer[MACFrame_FRAME_MAX_SIZE], data_buffer_size = MACFrame_FRAME_MAX_SIZE;
			if ( Routing_RecvDataTransferRequest(
					&_Pointer_MACHeader, rx_config->rx_buffer, data_buffer, &data_buffer_size )
						== Routing_SUCCESS && data_buffer_size > 0 )
			{
				UserPack upack;
				upack.FCmd.cmd = UserPack_Cmd_Data;
				upack.SCmd.devID = data_buffer[MACFrame_SOURCE_ADDRESS_OFFSET];
				upack.TotalSize = data_buffer_size - MACFrame_HEADER_SIZE - MACFrame_FCS_SIZE;
				UserPack_SetData(&upack, &(data_buffer[MACFrame_PAYLOAD_OFFSET]));
				USARTHandler_Send(&upack);
			}
		} break;
		// ---------------------------------------------------------------------------------------------------
		// Try to get the token back
		case (MACFrame_Flags_TOKEN | MACFrame_Flags_RET):
		{
			Routing_GetReturnedToken(rx_config->rx_buffer);
		} break;
		// ---------------------------------------------------------------------------------------------------
		default:
		{ // nop
		} break;		
		// ---------------------------------------------------------------------------------------------------

	} // switch ( rx_config->rx_buffer[MACFrame_FLAGS_OFFSET] )
}



void SWM1000_Initialization()
{
	peripherals_init();
	USARTHandler_Initialization();

	// timer for receiving config
	GeneralTimer_SetPrescaler(_Config_Timer, SystemCoreClock / 1000); // ms
	GeneralTimer_SetPeriod(_Config_Timer, SWM1000_ConfigRequestDelay);
	GeneralTimer_SetEvent(_Config_Timer); // first of all - send notification to driver
	GeneralTimer_Enable(_Config_Timer);

	// timeout timer for receiving config
	GeneralTimer_SetPrescaler(_ConfigTO_Timer, SystemCoreClock / 1000); // ms
	GeneralTimer_SetPeriod(_ConfigTO_Timer, SWM1000_ConfigRecvTimeOut);
	GeneralTimer_Reset(_ConfigTO_Timer);
	GeneralTimer_Enable(_ConfigTO_Timer);

	UserPack upack;
	do { // config from high level
		if (GeneralTimer_GetState(_Config_Timer) == GeneralTimer_SET) {
			GeneralTimer_Reset(_Config_Timer);
			upack.FCmd.cmd = UserPack_Cmd_SystemConfig;
			upack.SCmd._raw = 0;
			upack.TotalSize = 0;
			UHRes = USARTHandler_Send(&upack);
			if (UHRes == USARTHandler_ERROR)
				NVIC_SystemReset();
		}

		if (USARTHandler_isAvailableToReceive() == USARTHandler_TRUE) {
			UHRes = USARTHandler_Receive(&upack);

			if (UHRes == USARTHandler_SUCCESS &&
				upack.FCmd.cmd == UserPack_Cmd_SystemConfig &&
				upack.SCmd._raw == 0 &&
				upack.TotalSize == ConfigFW_SIZE
			) {
				UHRes = USARTHandler_ERROR; // just reset

				// data won't touched
				upack.FCmd.res = UserPack_CmdRes_Success;
				upack.SCmd.cmd = UserPack_Cmd_SystemConfig;
				upack.TotalSize = 0;
				USARTHandler_Send(&upack);
				break;
			}
		}
	} while (GeneralTimer_GetState(_ConfigTO_Timer) != GeneralTimer_SET);

	if (GeneralTimer_GetState(_ConfigTO_Timer) == GeneralTimer_SET) {
		_SWM1000_SetDefaultConfiguration();
	} else {
		ConfigFW_FromUserPack( &upack ); // data saved
	}
	_SWM1000_Configurate();

	GeneralTimer_Disable(_ConfigTO_Timer);
	GeneralTimer_Reset(_ConfigTO_Timer);
	GeneralTimer_Disable(_Config_Timer);
	GeneralTimer_Reset(_Config_Timer);

	// timer for generating new token
	GeneralTimer_SetPrescaler(_Generate_Timer, SystemCoreClock / 1000); // ms
	GeneralTimer_SetPeriod(_Generate_Timer, ConfigFW.SW1000.PollingPeriod * 2);
	// timer for polling
	GeneralTimer_SetPrescaler(_Poll_Timer, SystemCoreClock / 1000); // ms
	GeneralTimer_SetPeriod(_Poll_Timer, ConfigFW.SW1000.PollingPeriod);

	// watchdog settings - once at the power on
	IWDG_SetPrescaler(IWDG_Prescaler_32); // 1.25 kHz
	IWDG_SetReload(ConfigFW.SW1000.PollingPeriod & 0x0FFF);
	IWDG_ReloadCounter();
#ifndef DEBUG
	IWDG_Enable();
#endif

	GeneralTimer_Reset(_Generate_Timer);
	GeneralTimer_Enable(_Generate_Timer);
	GeneralTimer_Reset(_Poll_Timer);
	GeneralTimer_Enable(_Poll_Timer);
}



static inline void _SWM1000_Configurate()
{
	_Pointer_MACHeader.FrameControl = _SWM1000_FRAME_CONTROL;
	_Pointer_MACHeader.SequenceNumber = 0;
	_Pointer_MACHeader.PAN_ID = ConfigFW.SW1000.PAN_ID;
	_Pointer_MACHeader.SourceID = ConfigFW.SW1000.DeviceID & 0x00FF;

	Transceiver_Initialization();

	// Initialises network params
	dwt_setpanid(_Pointer_MACHeader.PAN_ID);
	dwt_setaddress16(_Pointer_MACHeader.SourceID);
	dwt_enableframefilter(_SWM1000_FRAME_FILTER_MASK);

	Routing_InitializationStruct routing_initializationStruct = {
		ConfigFW.SW1000.DeviceID,
		ConfigFW.Token.TimeSlotDurationMs * ConfigFW.SW1000.nDevices * 2,
		ConfigFW.Routing.MinSignalLevel
	};

	Ranging_Initialization(ConfigFW.Ranging.RespondingDelay, ConfigFW.Ranging.FinalDelay);
	TokenExt_Initialization(ConfigFW.SW1000.DeviceID, ConfigFW.SW1000.nDevices, ConfigFW.Token.TimeSlotDurationMs);
	Routing_Initialization(&routing_initializationStruct);
	Debugger_SetMode(ConfigFW.SW1000.DebugMode);
}



static inline void _SWM1000_SetDefaultConfiguration()
{
	uint8_t devID = 10;

	ConfigFW.Ranging.FinalDelay = 480;
	ConfigFW.Ranging.RespondingDelay = 500;
	ConfigFW.Routing.MinSignalLevel = 170;
	ConfigFW.SW1000.DebugMode = 0;
	ConfigFW.SW1000.DeviceID = devID;
	ConfigFW.SW1000.PAN_ID = 6896;
	ConfigFW.SW1000.PollingPeriod = 500;
	ConfigFW.SW1000.nDevices = 10;
	ConfigFW.Token.TimeSlotDurationMs = 1;
}



static UserPack_CommandRes _SWM1000_SendDistance()
{
	GeneralTimer_Reset(_Poll_Timer);

	uint8 i;
	UserPack_CommandRes result = UserPack_CmdRes_Fail;

	if (_Distance_UserPack.SCmd.devID == 0xFF) {
		for (i = 1; i <= ConfigFW.SW1000.nDevices; ++i) {
			if ( (uint8_t)(_Pointer_MACHeader.SourceID) != i ) {
				_Pointer_MACHeader.DestinationID = i & 0x00FF;
				_Pointer_MACHeader.Flags = MACFrame_Flags_NOP;
				RngRes = Ranging_Initiate(&_Pointer_MACHeader, _Distance_UserPack.Data);
				if (RngRes == Ranging_INTERRUPT)
					break;
			}
		}
	} else {
		_Pointer_MACHeader.DestinationID = _Distance_UserPack.SCmd.devID & 0x00FF;
		_Pointer_MACHeader.Flags = MACFrame_Flags_NOP;
		RngRes = Ranging_Initiate(&_Pointer_MACHeader, _Distance_UserPack.Data);
	}
	//
	if (RngRes == Ranging_INTERRUPT) {
		TokenExt_Reset();
	}
	result = UserPack_CmdRes_Success;

	return result;
}



static void _SWM1000_SetDefaultDistanceParams()
{
	_Distance_UserPack.FCmd.cmd = UserPack_Cmd_Distance;
	_Distance_UserPack.SCmd.devID = 0xFF;
	_Distance_UserPack.TotalSize = Ranging_PAYLOAD_SIZE;
	memset(_Distance_UserPack.Data, '0', Ranging_PAYLOAD_SIZE);
}



static void _SWM1000_SetDistanceParams(const UserPack *upack)
{
	_Distance_UserPack = *upack;
}



static inline void _SWM1000_PutDistanceInUData(uint8 *payload, uint16_str distance)
{		
	payload[Ranging_PAYLOAD_SIZE] = distance.b[0];
	payload[Ranging_PAYLOAD_SIZE + 1] = distance.b[1];
}


