
#include "_DEBUG.h"
#include "SWM1000.h"
#include "Transceiver.h"
#include "MACFrame.h"
#include "Ranging.h"
#include "USARTHandler.h"
#include "Random.h"
#include "Token.h"
#include "ConfigFW.h"



typedef union {
	uint16 d;
	uint8 b[2];
} uint16_str;



#define PREAMBLE_TIMEOUT				8




#define SWM1000_FRAME_CONTROL			0x8841		// ckeck IEEE Std 802.15.4-2011 for details
#define SWM1000_FRAME_FILTER_MASK		(DWT_FF_DATA_EN) // ckeck IEEE Std 802.15.4-2011 for details



static MACHeader_Typedef _Pointer_MACHeader;
static Ranging_RESULT RngRes;
static Token_RESULT TokRes;
static USARTHandler_RESULT UHRes;
//static Transceiver_RESULT TrRes;



//static uint8_t _SWM1000_WaitingInterrupt(void);
static inline void _SWM1000_PutDistanceInUData(uint8 *payload, uint16_str distance);
static inline void _SWM1000_TransceivingAutomat();
static inline void _SWM1000_ReceivingAutomat(Transceiver_RxConfig *rx_config);



void SWM1000_Initialization()
{
	UserPack upack;	
	USARTHandler_Initialization();
	
	do {
		// get own ID from higher level
		while (USARTHandler_isAvailableToReceive() == USARTHandler_FALSE)
			;
		UHRes = USARTHandler_Receive(&upack);
		
		if (UHRes == USARTHandler_SUCCESS && 
			upack.Command == UserPack_Cmd_SetConfig && 
			upack.TotalSize == ConfigFW_SIZE
		) {
			UHRes = USARTHandler_ERROR; // just reset	
			
			ConfigFW_FromUserPack(&upack);
			
			uint8 buf[] = "STARTED\n";
			upack.TotalSize = sizeof(buf);
			UserPack_SetData(&upack, buf);
		
#ifdef MAIN_DEBUG
/* TODO: testing space. Include testing code here */			
			USART_SendString("STARTED\n");
/* TODO: end of testing code*/		
#else
			USARTHandler_Send(&upack);
#endif
			
			break;
		}
	} while (1);
	
	_Pointer_MACHeader.FrameControl = SWM1000_FRAME_CONTROL;
	_Pointer_MACHeader.SequenceNumber = 0;
	_Pointer_MACHeader.PAN_ID = ConfigFW.SW1000.PAN_ID;
	_Pointer_MACHeader.SourceID = ConfigFW.SW1000.DeviceID & 0x00FF;
	
	Transceiver_Initialization();
	
	// Initialises network params
	dwt_setpanid(_Pointer_MACHeader.PAN_ID);
	dwt_setaddress16(_Pointer_MACHeader.SourceID);
	dwt_enableframefilter(SWM1000_FRAME_FILTER_MASK);
		
	Random_Initialization();
	Ranging_Initialization(ConfigFW.Ranging.RespondingDelay, ConfigFW.Ranging.FinalDelay);
	Token_Initialization(ConfigFW.SW1000.nDevices, ConfigFW.Token.TimeSlotDurationMs);
	
	// timer for 
	GeneralTimer_SetPrescaler(SystemCoreClock / 1000); // ms
	GeneralTimer_SetPeriod(ConfigFW.SW1000.PollingPeriod);		
}



void SWM1000_Loop(void)
{
	uint8 buffer[UserPack_MAX_DATA_SIZE];
	uint8 generate, transmit, token_transfer; 
	uint8 receivingState = 0;
	Transceiver_RxConfig rx_config;
	rx_config.rx_buffer = buffer;
	rx_config.rx_buffer_size = UserPack_MAX_DATA_SIZE;
	UserPack upack;
		
	GeneralTimer_Reset();
	GeneralTimer_Enable();
	
	while (1) 
	{
		// receive data from HL
		if ( USARTHandler_isAvailableToReceive() == USARTHandler_TRUE ) 
		{	
			UHRes = USARTHandler_Receive(&upack);			

#ifdef MAIN_DEBUG	
/* TODO: testing space. Include testing code here */			
			if (UHRes == USARTHandler_SUCCESS)		
				USART_SendString("HL data recvd\n");
			else
				USART_SendString("HL data error\n");
/* TODO: end of testing code*/
#endif	
	
		}	
		
		generate = (GeneralTimer_GetState() == GeneralTimer_SET);
		token_transfer = (Token_isCaptured() == Token_TRUE);
		transmit = (token_transfer && UHRes == USARTHandler_SUCCESS);
		
		if (generate || token_transfer || transmit) {
			// reset
			receivingState = 0;
			Transceiver_ReceiverOff();	
			// generate token in the absence other requests
			if (generate) {	
				_Pointer_MACHeader.DestinationID = 0xFFFF;			
				TokRes = Token_Generate( &_Pointer_MACHeader );
				//deca_sleep(10);
				GeneralTimer_Reset();
				
#ifdef MAIN_DEBUG		
/* TODO: testing space. Include testing code here */				
				USART_SendString("token gen\n");
/* TODO: end of testing code*/
#endif	
	
			}
			// if transfer is allowed
			if (transmit) {
				UHRes = USARTHandler_ERROR; // just reset	
				_SWM1000_TransceivingAutomat(&upack);
			}
			// if token is captured
			if (token_transfer) {
				//deca_sleep(10);
				_Pointer_MACHeader.DestinationID = 0xFFFF;	
				TokRes = Token_Transfer( &_Pointer_MACHeader );
				
#ifdef MAIN_DEBUG
/* TODO: testing space. Include testing code here */				
				if (TokRes == Token_SUCCESS) {
					USART_SendString("tok_tx success\n");					
				} else {				
					USART_SendString("tok_tx fail\n");
				}
/* TODO: end of testing code*/
#endif
				
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
						rx_config.rx_buffer_size = Transceiver_GetAvailableData(rx_config.rx_buffer); // read
						_SWM1000_ReceivingAutomat( &rx_config ); // respond
						receivingState = 0; // reset
						GeneralTimer_Reset();
					}
				} break;
			}
			
		}				
	} // while (1)
}



static inline void _SWM1000_TransceivingAutomat(UserPack *upack)
{
	uint8 i;
	switch (upack->Command) {
		// -------------------------------------------------------------------------
		case UserPack_Cmd_Distance: 
		{	
			if (upack->DestinationID == 0xFF) {
				for (i = 1; i <= ConfigFW.SW1000.nDevices; ++i) {							
					if ( (uint8_t)(_Pointer_MACHeader.SourceID) != i ) {
			
#ifdef MAIN_DEBUG
/* TODO: testing space. Include testing code here */
						USART_SendString("rang init\n");
/* TODO: end of testing code*/
#endif	
						
						_Pointer_MACHeader.DestinationID = i & 0x00FF;
						RngRes = Ranging_Initiate(&_Pointer_MACHeader, upack->Data);		
					}
				}	
			} else {
				_Pointer_MACHeader.DestinationID = upack->DestinationID & 0x00FF;
				RngRes = Ranging_Initiate(&_Pointer_MACHeader, upack->Data);	
			}
		} break;
		// -------------------------------------------------------------------------		
		default:
		{ // nop
		} break;
		// -------------------------------------------------------------------------	
		
	} // switch (upack->Command)
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
				UserPack upack;
				upack.DestinationID = rx_config->rx_buffer[MACFrame_SOURCE_ADDRESS_OFFSET];
				upack.Command = UserPack_Cmd_Distance;
				_SWM1000_PutDistanceInUData(&(rx_config->rx_buffer[MACFrame_PAYLOAD_OFFSET]), distance);
				upack.TotalSize = Ranging_PAYLOAD_SIZE + 2; // + 2 for distance
				UserPack_SetData(&upack, &(rx_config->rx_buffer[MACFrame_PAYLOAD_OFFSET]));		
				
#ifdef MAIN_DEBUG	
/* TODO: testing space. Include testing code here */				
				USART_SendString("distance recvd\n");
/* TODO: end of testing code*/ 
#else
				USARTHandler_Send(&upack);
#endif	
	
			}
		} break;						
		// -------------------------------------------------------------------------			
		case MACFrame_Flags_TOKEN:					
		{	
			_Pointer_MACHeader.DestinationID = 0xFFFF; // everybody should be able to hear this message			
			TokRes = Token_Receipt( &_Pointer_MACHeader, rx_config->rx_buffer[MACFrame_SOURCE_ADDRESS_OFFSET] );
			
#ifdef MAIN_DEBUG	
/* TODO: testing space. Include testing code here */				
			if (TokRes == Token_SUCCESS) {			
				USART_SendString("tok_recvd success\n");
			} else {			
				USART_SendString("tok_recvd fail\n");
			}
/* TODO: end of testing code*/ 	
#endif				
			
		} break;	
		// -------------------------------------------------------------------------			
		case (MACFrame_Flags_TOKEN | MACFrame_Flags_ACK):					
		{				
			if (Token_isCaptured() == Token_TRUE) {
				_Pointer_MACHeader.DestinationID = 0xFFFF;
				Token_ImmediateReceipt(&_Pointer_MACHeader);	
			}

#ifdef MAIN_DEBUG	
/* TODO: testing space. Include testing code here */			
			USART_SendString("TOKEN | ACK\n");
/* TODO: end of testing code*/ 
#endif	
			
		} break;		
		// -------------------------------------------------------------------------	
		default:
		{ // nop
		} break;		
		// -------------------------------------------------------------------------	
	}
}



/*static uint8_t _SWM1000_WaitingInterrupt(void)
{
	return USARTHandler_isAvailableToReceive();
		
}*/



static inline void _SWM1000_PutDistanceInUData(uint8 *payload, uint16_str distance)
{		
	payload[Ranging_PAYLOAD_SIZE] = distance.b[0];
	payload[Ranging_PAYLOAD_SIZE + 1] = distance.b[1];
}


