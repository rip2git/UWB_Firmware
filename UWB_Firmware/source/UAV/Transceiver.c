
//#include "Debugger.h"
#include "Transceiver.h"
#include <math.h>


// callbacks result (defines IRQ event)
#define cbRes 	decairq_callBackResult



// 50/50
#define SNIFF_ON_TIME 	1		// PACs (hard + 1, then if PAC = 8 -> 8*2 = 16 us)
#define SNIFF_OFF_TIME 	4  		// us



static inline void _Transceiver_rxoff(void)
{
	dwt_receiverautoreenabled(0);
	dwt_forcetrxoff();
    dwt_rxreset();
}



Transceiver_RESULT Transceiver_Transmit(Transceiver_TxConfig *config)
{
	uint32 frame_len;
	uint32 currentMASK;
	uint8 flags = DWT_START_TX_IMMEDIATE; // 0
	
	decamutexon();
	
	// Disable current events
	dwt_setinterrupt(SYS_MASK_MASK_32, 0);
	if (config->rx_aftertx_delay) // Response expected -> Enable transceiver events - receive / TO / errors
		currentMASK = SYS_MASK_MRXFCG | SYS_MASK_MRXRFTO | SYS_MASK_MRXPHE | SYS_MASK_MRXFCE | SYS_MASK_MAFFREJ;
	else // Enable transceiver events - sent
		currentMASK = SYS_MASK_MTXFRS;

	dwt_setinterrupt(currentMASK, 1);
	
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
		while ( !(cbRes & currentMASK) )
			;			

		_Transceiver_rxoff(); // reset state
		
		cbRes = (cbRes & Transceiver_RXE)? Transceiver_RXE : cbRes & currentMASK;

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
		while ( !(cbRes & currentMASK) )
			;
		cbRes &= currentMASK;
	}	
	
	return cbRes;
}



Transceiver_RESULT Transceiver_Receive(Transceiver_RxConfig *config)
{
	uint32 frame_len;
	uint32 currentMASK = SYS_MASK_MRXFCG | SYS_MASK_MRXRFTO | SYS_MASK_MRXPHE | SYS_MASK_MRXFCE | SYS_MASK_MAFFREJ;
	
	decamutexon();
	
	// Disable current events
	dwt_setinterrupt(SYS_MASK_MASK_32, 0);
	// Enable transceiver events - receive / TO / errors
	dwt_setinterrupt(currentMASK, 1);
	
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
		
	// Waiting message / TO / errors
	while ( !(cbRes & currentMASK) ) {
		if (config->rx_interrupt != NULL && config->rx_interrupt() != 0) { // interrupts the process if needed
			_Transceiver_rxoff();
			return Transceiver_INTERRUPTED;
		}
	}	

	_Transceiver_rxoff(); // reset state

	cbRes = (cbRes & Transceiver_RXE)? Transceiver_RXE : cbRes & currentMASK;

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
	


void Transceiver_ReceiverOn()
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



void Transceiver_ReceiverOff(void)
{		
	_Transceiver_rxoff();
	//dwt_setsniffmode(0, 0, 0);
}



		
Transceiver_RESULT Transceiver_GetReceptionResult(void)
{
	return (cbRes & Transceiver_RXFCG);
}
	
	
	
uint8 Transceiver_GetAvailableData(uint8 *buffer)
{
	if ( (cbRes & Transceiver_RXFCG) ) {
		// reset state
		Transceiver_ReceiverOff();
		
		cbRes = 0; // reset		
		
		uint32 frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
		dwt_readrxdata(buffer, frame_len, 0);
		return (uint8)frame_len;
	}
	return 0;
}



uint8 Transceiver_GetLevelOfLastReceived(void)
{
	uint16 RXPACC = (dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXPACC_MASK) >> RX_FINFO_RXPACC_SHIFT;
	uint16 CIR_PWR = dwt_read16bitoffsetreg(RX_FQUAL_ID, 6);
	// experemental original info: 24 -min, 45 -max (10.0 * log10(x)))
	// theoretical: from 0 to 125;
	// lets return from 0 to 250
	return (uint8)( 5.0 * 10.0 * log10( (double)(CIR_PWR * 1 << 17) / (double)(RXPACC * RXPACC) ) );
//	return (uint8)(( 20.0 * log10( (double)(CIR_PWR * 1 << 17) / (double)(RXPACC * RXPACC) ) - 40.0 ) * 2.0);
}



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

    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_CPLOCK | SYS_STATUS_SLP2INIT | SYS_STATUS_CLKPLL_LL);
}


