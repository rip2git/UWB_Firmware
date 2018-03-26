
#include "_DEBUG.h"
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



#define PREAMBLE_TIMEOUT				8



#define SWM1000_PAN_ID					0x1AF0		// any creativity
#define SWM1000_FRAME_CONTROL			0x8841		// ckeck IEEE Std 802.15.4-2011 for details
#define SWM1000_FRAME_FILTER_MASK		(DWT_FF_DATA_EN) // ckeck IEEE Std 802.15.4-2011 for details
#define SWM1000_MAXIMUM_BUFFER_SIZE		127



static MACHeader_Typedef _Pointer_MACHeader;
//static Transceiver_RESULT TrRes;
static Ranging_RESULT RngRes;



static Token_RESULT TokRes;



static USARTHandler_RESULT UHRes;
static uint8 buffer[SWM1000_MAXIMUM_BUFFER_SIZE];



//static uint8_t _SWM1000_WaitingInterrupt(void);
static inline void _SWM1000_PutDistanceInUData(uint8 *payload, uint16_str distance);
static inline void _SWM1000_TransceivingAutomat();
static inline void _SWM1000_ReceivingAutomat(Transceiver_RxConfig *rx_config);



void SWM1000_Initialization()
{
	UserPack upack;
	Transceiver_Initialization();	
	USARTHandler_Initialization();
	
	// get own ID from higher level
	UHRes = USARTHandler_Receive(&upack);
	
	if (UHRes == USARTHandler_SUCCESS && upack.Command == UserPack_Cmd_SetID) {
		UHRes = USARTHandler_ERROR; // just reset	
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
		
	}
	
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
	
	// timer for 
	GeneralTimer_SetPrescaler(SystemCoreClock / 1000); // ms
	GeneralTimer_SetPeriod(SWM1000_PollingPeriod);		
}



void SWM1000_Loop(void)
{
	uint8 generate, transmit, token_transfer; 
	uint8 receivingState = 0;
	Transceiver_RxConfig rx_config;
	rx_config.rx_buffer = buffer;
	rx_config.rx_buffer_size = SWM1000_MAXIMUM_BUFFER_SIZE;
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
				if (TokRes == Token_SUCCESS) {
					
#ifdef MAIN_DEBUG
/* TODO: testing space. Include testing code here */					
					USART_SendString("tok_tx success\n");
/* TODO: end of testing code*/
#endif	
					
				} else {
					
#ifdef MAIN_DEBUG	
/* TODO: testing space. Include testing code here */					
					USART_SendString("tok_tx fail\n");
/* TODO: end of testing code*/
#endif	
					
				}
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
				for (i = 1; i <= SWM1000_nDEVICES; ++i) {							
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
			if (TokRes == Token_SUCCESS) {
			
#ifdef MAIN_DEBUG	
/* TODO: testing space. Include testing code here */				
				USART_SendString("tok_recvd success\n");
/* TODO: end of testing code*/ 
#endif	
				
			} else {
			
#ifdef MAIN_DEBUG	
/* TODO: testing space. Include testing code here */				
				USART_SendString("tok_recvd fail\n");
/* TODO: end of testing code*/ 	
#endif				
				
			}
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


