
#include "Transceiver.h"


// callbacks result (defines IRQ event)
#define cbRes 	decairq_callBackResult



// 50/50
#define SNIFF_ON_TIME 	1		// PACs (hard + 1, then if PAC = 8 -> 8*2 = 16 us)
#define SNIFF_OFF_TIME 	4  		// us



void Transceiver_Initialization(void)
{	
	dwt_config_t config = {
		2,               /* Channel number. */
		DWT_PRF_64M,     /* Pulse repetition frequency. */
		DWT_PLEN_128,    /* Preamble length. Used in TX only. */
		DWT_PAC8,        /* Preamble acquisition chunk size. Used in RX only. */
		9,               /* TX preamble code. Used in TX only. */
		9,               /* RX preamble code. Used in RX only. */
		0,               /* 0 to use standard SFD, 1 to use non-standard SFD. */
		DWT_BR_6M8,      /* Data rate. */
		DWT_PHRMODE_STD, /* PHY header mode. */
		(129 + 8 - 8)    /* SFD timeout (preamble length + 1 + SFD length - PAC size). Used in RX only. */
	};
	
	spi_set_rate_low();
	do {
		deca_sleep(500);
		reset_DW1000();
	} while ( dwt_initialise(DWT_LOADUCODE) == DWT_ERROR );
    
	spi_set_rate_high();    
    dwt_configure(&config);	
}



static inline void _Transceiver_rxoff(void)
{
	dwt_receiverautoreenabled(0);
	dwt_forcetrxoff();
    dwt_rxreset();
}



Transceiver_RESULT Transceiver_Transmit(Transceiver_TxConfig *config)
{
	uint32 frame_len;
	uint8 flags = DWT_START_TX_IMMEDIATE; // 0
	
	decamutexon();
	
	// Disable current events
	dwt_setinterrupt(SYS_MASK_MASK_32, 0);
	if (config->rx_aftertx_delay) // Response expected -> Enable transceiver events - receive, rx TO
		dwt_setinterrupt(SYS_MASK_MRXFCG | SYS_MASK_MRXRFTO, 1);
	else // Enable transceiver events - sent, receive, rx TO	
		dwt_setinterrupt(SYS_MASK_MTXFRS, 1);
	
	cbRes = 0;
	
	decamutexoff();
	
	if (config->rx_aftertx_delay) {
		dwt_receiverautoreenabled(1);
		dwt_setrxaftertxdelay(config->rx_aftertx_delay);		
		dwt_setrxtimeout(config->rx_timeout);
		flags |= DWT_RESPONSE_EXPECTED;
	}	
	
	if (config->tx_delay) {
		dwt_setdelayedtrxtime(config->tx_delay);
		flags |= DWT_START_TX_DELAYED;
	}
	
	// Zero offset in TX buffer.
	dwt_writetxdata(config->tx_buffer_size, config->tx_buffer, 0);
	dwt_writetxfctrl(config->tx_buffer_size, 0, config->ranging);
		
	DWT_RESULT res = dwt_starttx(flags);
	if ( res == DWT_ERROR ) {
		return Transceiver_ERROR;		
	}
	
	if (config->rx_aftertx_delay) { // Response expected
		while ( !(cbRes & (Transceiver_RXFCG | Transceiver_RXRFTO)) )
			;			

		_Transceiver_rxoff(); // reset state
		
		if (cbRes == Transceiver_RXFCG) {
			// A frame has been received, read it into the buffer
			frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
			if (frame_len <= config->rx_buffer_size) {
				config->rx_buffer_size = frame_len;
				dwt_readrxdata(config->rx_buffer, frame_len, 0);
			} else {
				cbRes = Transceiver_ERROR;
			}			
		}
	} 
	else { // Response isn't expected		
		while ( !(cbRes & Transceiver_TXFRS) )
			;
	}	
	
	return cbRes;
}



Transceiver_RESULT Transceiver_Receive(Transceiver_RxConfig *config)
{
	uint32 frame_len;
	
	decamutexon();
	
	// Disable current events
	dwt_setinterrupt(SYS_MASK_MASK_32, 0);
	// Enable transceiver events - receive, rx TO
	dwt_setinterrupt(SYS_MASK_MRXFCG | SYS_MASK_MRXRFTO, 1);
	
	cbRes = 0;
	
	decamutexoff();
	
	dwt_receiverautoreenabled(1);
	dwt_setrxtimeout(config->rx_timeout);
	
	if (config->rx_delay) {		
		dwt_setdelayedtrxtime(config->rx_delay);
		dwt_rxenable(DWT_START_RX_DELAYED);		
	} else {
		dwt_setdelayedtrxtime(0);
		dwt_rxenable(DWT_START_RX_IMMEDIATE);
	}
		
	// Waiting message or timeout	
	while ( !(cbRes & (Transceiver_RXFCG | Transceiver_RXRFTO)) ) {		
		if (config->rx_interrupt != NULL) { // interrupts the process if needed
			if (config->rx_interrupt() != 0) {
				_Transceiver_rxoff();
				return Transceiver_INTERRUPTED;
			}
		}
	}	
	_Transceiver_rxoff(); // reset state
	

	if (cbRes == Transceiver_RXFCG) {
		// A frame has been received, read it into the buffer
		frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
		if (frame_len <= config->rx_buffer_size) {
			config->rx_buffer_size = frame_len;
			dwt_readrxdata(config->rx_buffer, frame_len, 0);
		} else {
			cbRes = Transceiver_ERROR;
		}		
	}
	
	return cbRes;
}



Transceiver_RESULT Transceiver_ListenEnvironment(uint16 timeout)
{	
	decamutexon();
	
	// Disable current events
	dwt_setinterrupt(SYS_MASK_MASK_32, 0);
	// Enable preamble events
	dwt_setinterrupt(SYS_MASK_MRXPRD | SYS_MASK_MRXPTO, 1);
	
	cbRes = 0;
	
	decamutexoff();
	
	dwt_setpreambledetecttimeout(timeout);
	dwt_rxenable(DWT_START_RX_IMMEDIATE);

	while ( !(cbRes & (Transceiver_RXPRD | Transceiver_RXPTO)) )
		;
	_Transceiver_rxoff(); // reset state
		
	dwt_setpreambledetecttimeout(0); // reset
	return cbRes;
}
	


inline void Transceiver_ReceiverOn()
{
	// Disable current events
	dwt_setinterrupt(SYS_MASK_MASK_32, 0);
	// Enable transceiver events - receive
	dwt_setinterrupt(SYS_MASK_MRXFCG, 1);
	
	cbRes = 0;
	
	//dwt_setsniffmode(1, SNIFF_ON_TIME, SNIFF_OFF_TIME);
	
	dwt_setrxtimeout(0);
	dwt_setdelayedtrxtime(0);
	dwt_receiverautoreenabled(1);
	dwt_rxenable(DWT_START_RX_IMMEDIATE);
}



inline void Transceiver_ReceiverOff(void)
{		
	_Transceiver_rxoff();
	//dwt_setsniffmode(0, 0, 0);
}


		
inline Transceiver_RESULT Transceiver_GetReceptionResult(void)
{
	return cbRes;
}
	
	
	
uint8 Transceiver_GetAvailableData(uint8 *buffer)
{
	if ( cbRes == Transceiver_RXFCG ) {
		// reset state
		Transceiver_ReceiverOff();
		
		cbRes = 0; // reset		
		
		uint32 frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
		dwt_readrxdata(buffer, frame_len, 0);
		return (uint8)frame_len;
	}
	return 0;
}


