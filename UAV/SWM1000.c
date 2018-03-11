
#include "SWM1000.h"
#include "Transceiver.h"
#include "MACFrame.h"
#include "Ranging.h"
#include "USARTHandler.h"
#include "Random.h"
#include "Token.h"



typedef union {
	uint16 d;
	uint8 b[2];
} uint16_str;



#define RX_NORMAL_TIMEOUT				5000 // us
#define PREAMBLE_TIMEOUT				8



#define SWM1000_PAN_ID					0x1AF0		// any creativity
#define SWM1000_FRAME_CONTROL			0x8841		// ckeck IEEE Std 802.15.4-2011 for details
#define SWM1000_FRAME_FILTER_MASK		(DWT_FF_DATA_EN) // ckeck IEEE Std 802.15.4-2011 for details
#define SWM1000_MAXIMUM_BUFFER_SIZE		127



static MACHeader_Typedef _Pointer_MACHeader;
static Transceiver_RESULT TrRes;
static Ranging_RESULT RngRes;



static Token_RESULT TokRes;



static UserPack upack;
static USARTHandler_RESULT UHRes;
static uint8 buffer[SWM1000_MAXIMUM_BUFFER_SIZE];



static uint8_t _SWM1000_WaitingInterrupt(void);
static inline void _SWM1000_PutDistanceInUData(uint8 *payload, uint16_str distance);
static inline void _SWM1000_TransceivingAutomat();
static inline void _SWM1000_ReceivingAutomat(Transceiver_RxConfig *rx_config);



void SWM1000_Initialization()
{
	Transceiver_Initialization();	
	USARTHandler_Initialization();
	
	// get own ID from higher level
	UHRes = USARTHandler_Receive(&upack);
	
	if (UHRes == USARTHandler_SUCCESS && upack.Command == UserPack_Cmd_SetID) {
		uint8 buf[] = "STARTED\n";
		upack.TotalSize = sizeof(buf);
		UserPack_SetData(&upack, buf);
		USARTHandler_Send(&upack);
	}
	//upack.DestinationID = 4;
	
	// Initialises network params
	dwt_setpanid(SWM1000_PAN_ID);
	dwt_setaddress16(upack.DestinationID);
	dwt_enableframefilter(SWM1000_FRAME_FILTER_MASK);
	
	_Pointer_MACHeader.FrameControl = SWM1000_FRAME_CONTROL;
	_Pointer_MACHeader.SequenceNumber = 0;
	_Pointer_MACHeader.PAN_ID = SWM1000_PAN_ID;
	_Pointer_MACHeader.SourceID = upack.DestinationID & 0x00FF;
	
	Ranging_Initialization();
	Random_Initialization();
	Token_SetMaxID(SWM1000_nDEVICES);
}



volatile uint32 SWM1000_tmp_vola[4];
void SWM1000_Loop(void)
{		
	uint8 generate, transmit, token_transfer; 
	uint8 receivingState = 0;
	Transceiver_RxConfig rx_config;
	rx_config.rx_buffer = buffer;
	rx_config.rx_buffer_size = SWM1000_MAXIMUM_BUFFER_SIZE;
	uint32 pollingStart = portGetTickCount();
	
	while (1) 
	{
		// receive data from HL
		if ( USARTHandler_isAvailableToReceive() ) 
		{	
			UHRes = USARTHandler_Receive(&upack);
		}	
		
		generate = (portGetTickCount() - pollingStart > SWM1000_PollingPeriod);
		token_transfer = (Token_isCaptured() == Token_TRUE);
		transmit = (token_transfer && UHRes == USARTHandler_SUCCESS);
		
		if (generate || token_transfer || transmit) {
			// reset
			receivingState = 0;
			Transceiver_ReceiverOff();	
			// generate token in the absence other requests
			if (generate) {
				pollingStart = portGetTickCount(); // reset
				_Pointer_MACHeader.DestinationID = 0xFFFF;			
				TokRes = Token_Generate( &_Pointer_MACHeader );
				sleep_10ms(1);
				if (TokRes == Token_SUCCESS)
					SWM1000_tmp_vola[0]++;				
			}
			// if transfer is allowed
			if (transmit) {
				UHRes = USARTHandler_ERROR; // just reset	
				_SWM1000_TransceivingAutomat();	// upack is available inside
			}
			// if token is captured
			if (token_transfer) {
				sleep_10ms(1);
				_Pointer_MACHeader.DestinationID = 0xFFFF;	
				TokRes = Token_Transfer( &_Pointer_MACHeader );
				if (TokRes == Token_SUCCESS)
					SWM1000_tmp_vola[1]++;
			}
		}
		// receiving
		else { 
			
			switch (receivingState) {
				case 0: // receiver turn on
				{					
					Transceiver_ReceiverOn();
					receivingState = 1; // waiting request
				} break;
	
				case 1: // get data if it is available
				{
					if (Transceiver_GetReceptionResult() == Transceiver_RXFCG) {
						pollingStart = portGetTickCount(); // reset
						rx_config.rx_buffer_size = Transceiver_GetAvailableData(rx_config.rx_buffer); // read
						_SWM1000_ReceivingAutomat( &rx_config ); // respond
						receivingState = 0; // reset
					}
				} break;
			}
			
		}				
	} // while (1)
}



static inline void _SWM1000_TransceivingAutomat()
{
	uint8 i;
	switch (upack.Command) {
		// -------------------------------------------------------------------------
		case UserPack_Cmd_Distance: 
		{	
			if (upack.DestinationID == 0xFF) {
				for (i = 1; i <= SWM1000_nDEVICES; ++i) {							
					if ( (uint8_t)(_Pointer_MACHeader.SourceID) != i ) {									
						_Pointer_MACHeader.DestinationID = i & 0x00FF;
						RngRes = Ranging_Initiate(&_Pointer_MACHeader, upack.Data);		
					}
				}	
			} else {
				_Pointer_MACHeader.DestinationID = upack.DestinationID & 0x00FF;
				RngRes = Ranging_Initiate(&_Pointer_MACHeader, upack.Data);	
			}
		} break;
		// -------------------------------------------------------------------------		
		default:
		{ // nop
		} break;
		// -------------------------------------------------------------------------	
		
	} // switch (upack.Command)
}



static inline void _SWM1000_ReceivingAutomat(Transceiver_RxConfig *rx_config)
{
	uint16_str distance;	
	
	switch ( rx_config->rx_buffer[MACFrame_FLAGS_OFFSET] ) {
		// -------------------------------------------------------------------------	
		// Ranging algorithm - getting distance
		case MACFrame_Flags_RNG:					
		{					
			_Pointer_MACHeader.DestinationID = rx_config->rx_buffer[MACFrame_SOURCE_ADDRESS_OFFSET]; // low byte first	
			
			RngRes = Ranging_GetDistance( &_Pointer_MACHeader, &(distance.d) );
			if (RngRes == Ranging_SUCCESS) {
				upack.DestinationID = rx_config->rx_buffer[MACFrame_SOURCE_ADDRESS_OFFSET];
				upack.Command = UserPack_Cmd_Distance;
				_SWM1000_PutDistanceInUData(&(rx_config->rx_buffer[MACFrame_PAYLOAD_OFFSET]), distance);
				upack.TotalSize = Ranging_PAYLOAD_SIZE + 2; // + 2 for distance
				UserPack_SetData(&upack, &(rx_config->rx_buffer[MACFrame_PAYLOAD_OFFSET]));		
				
				//upack.TotalSize = Ranging_PAYLOAD_SIZE + 4;
				//UserPack_SetData(&upack, rx_config->rx_buffer);

				USARTHandler_Send(&upack);
			}
		} break;						
		// -------------------------------------------------------------------------			
		case MACFrame_Flags_TOKEN:					
		{	
			_Pointer_MACHeader.DestinationID = 0xFFFF; // everybody should be able to hear this message			
			TokRes = Token_Receipt( &_Pointer_MACHeader, rx_config->rx_buffer );
			if (TokRes == Token_SUCCESS)
				SWM1000_tmp_vola[2]++;
		} break;	
		// -------------------------------------------------------------------------			
		case (MACFrame_Flags_TOKEN | MACFrame_Flags_ACK):					
		{	
			SWM1000_tmp_vola[3]++;
		} break;		
		// -------------------------------------------------------------------------	
		default:
		{ // nop
		} break;		
		// -------------------------------------------------------------------------	
	}
}



static uint8_t _SWM1000_WaitingInterrupt(void)
{
	return USARTHandler_isAvailableToReceive();
		
}



static inline void _SWM1000_PutDistanceInUData(uint8 *payload, uint16_str distance)
{		
	payload[Ranging_PAYLOAD_SIZE] = distance.b[0];
	payload[Ranging_PAYLOAD_SIZE + 1] = distance.b[1];
}


