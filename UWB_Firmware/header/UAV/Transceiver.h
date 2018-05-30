#ifndef TRANSCEIVER_
#define TRANSCEIVER_


#include "deca_sleep.h"
#include "deca_device_api.h"
#include "deca_regs.h"
#include "port.h"



/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: Transceiver_RESULT
 *
 * @brief: Used for results (described below) returned from Transceiver methods
 *
*/
typedef enum {
	Transceiver_ERROR			= 0x00000000UL,		// Some types of error: rx/tx_buffer from config could not be used
	Transceiver_INTERRUPTED		= 0x00000001UL,		// if receiving was interrupted
	Transceiver_TXFRS			= SYS_STATUS_TXFRS,	// Transmit Frame Sent
	Transceiver_RXFCG			= SYS_STATUS_RXFCG,	// Receiver FCS Good (data frame ready)
	Transceiver_RXRFTO			= SYS_STATUS_RXRFTO,// Receive Frame Wait Timeout
	Transceiver_RXPRD			= SYS_STATUS_RXPRD,	// Receiver Preamble Detected
	Transceiver_RXPTO			= SYS_STATUS_RXPTO,	// Preamble detection timeout
	Transceiver_RXE				= (SYS_STATUS_RXFCE | SYS_STATUS_RXPHE | SYS_STATUS_AFFREJ) // FCS | PHY Header Error | Frame Filtering rejection
} Transceiver_RESULT;

/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: Transceiver_Interrupt
 *
 * @brief: Used for interrupt receiving from Transceiver_Receive method; contained in Transceiver_RxConfig,
 * described below.
 *
 * NOTE: 	return = 0 - do not interrupt the process
 * 			return != 0 - interrupt the process
 *
 * NOTE:	It requires to generate WFI or WFE to wakes up MC (not implemented now - works without WFI/WFE)
 *
 * NOTE:	Can be set as NULL - doesn't be used.
 *
 * NOTE:	Transceiver_Receive returns Transceiver_INTERRUPTED
 *
*/
typedef uint8_t (*Transceiver_Interrupt)(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: Transceiver_TxConfig struct
 *
 * @brief: Used as input parameter in Transceiver_Transmit method; content described below
 *
*/
typedef struct {
	uint8 	*tx_buffer;				// payload
	uint16 	tx_buffer_size;			// payload size in bytes
	uint32 	tx_delay;				// if needs to transmit the message in certain time (in uus)
	int 	ranging;				// provides ranging message
	// provides rx after tx event	
	uint32	rx_aftertx_delay;		// turns receiver on after transmit the message after that time (in uus)
	uint8 	*rx_buffer;				// buffer for expected response
	uint16 	rx_buffer_size;			// size of expected response in bytes
	uint16	rx_timeout;				// time for waiting of response (in uus)
} Transceiver_TxConfig;

/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: Transceiver_RxConfig struct
 *
 * @brief: Used as input parameter in Transceiver_Receive method; content described below
 *
*/
typedef struct {
	uint8 	*rx_buffer;					// buffer for expected message
	uint16 	rx_buffer_size;				// size of expected message in bytes, can be changed to less value
	uint16	rx_timeout;					// time for waiting of a message (in uus)
	uint32 	rx_delay;					// if needs to receive a message in certain time (in uus)
	Transceiver_Interrupt rx_interrupt;	// used to inerrupt receiving
} Transceiver_RxConfig;


/*! ------------------------------------------------------------------------------------------------------------------
 * @fn Transceiver_Initialization()
 *
 * @brief: Initialises device, callbacks, IRQ hanler and MC sleep function
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * no return value
*/
extern void Transceiver_Initialization(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn Transceiver_Transmit()
 *
 * @brief: Transmits the message from config param using the given rules from that config
 *
 * NOTE: 
 *
 * input parameters
 * @param config - not constant pointer to Transceiver_TxConfig described above, contained
 * 		rules for transmitting
 *
 * output parameters
 * @param config - not constant pointer to Transceiver_TxConfig described above; 
 * 		expected response can be get from that param
 *
 * return value is Transceiver_RESULT described above
*/
extern Transceiver_RESULT Transceiver_Transmit(Transceiver_TxConfig *config);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn Transceiver_Receive()
 *
 * @brief: Receiving the message using the given rules from config param
 *
 * NOTE: 
 *
 * input parameters
 * @param config - not constant pointer to Transceiver_RxConfig described above, contained
 * 		rules for receiving
 *
 * output parameters
 * @param config - not constant pointer to Transceiver_RxConfig described above;
 * 		expected message can be get from that param
 *
 * return value is Transceiver_RESULT described above
*/
extern Transceiver_RESULT Transceiver_Receive(Transceiver_RxConfig *config);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn Transceiver_ReceiverOn()
 *
 * @brief: 
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * no returned value
*/
extern void Transceiver_ReceiverOn(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn Transceiver_ReceiverOff()
 *
 * @brief: 
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * no returned value
*/
extern void Transceiver_ReceiverOff(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn Transceiver_GetReceptionResult()
 *
 * @brief: 
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * no returned value
*/
extern Transceiver_RESULT Transceiver_GetReceptionResult(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn Transceiver_GetLevelOfLastReceived()
 *
 * @brief: allows to get received signal strength of last received frame (from 0 to 250 cu)
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * return received signal strength
*/
extern uint8 Transceiver_GetLevelOfLastReceived(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn Transceiver_GetAvailableData()
 *
 * @brief:
 *
 * NOTE:
 *
 * input parameters
 * @param buffer
 *
 * output parameters
 * @param buffer
 *
 * return size of received data (size of buffer)
*/
extern uint8 Transceiver_GetAvailableData(uint8 *buffer);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn Transceiver_ListenEnvironment()
 *
 * @brief: Transceiver turns into listening of preamble
 *
 * NOTE: 
 *
 * input parameters
 * @param timeout - preamble detection timeout, after that time Transceiver is turn off
 *
 * output parameters
 *
 * return value is Transceiver_RESULT described above
*/
//extern Transceiver_RESULT Transceiver_ListenEnvironment(uint16 timeout);


#endif
