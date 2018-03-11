
#include "Transceiver.h"
#include "port.h"
#include "deca_sleep.h"


// callbacks result (defines IRQ event)
static volatile Transceiver_RESULT cbRes;

// IRQ callbacks
static void _Transceiver_cbRxOk(uint32 status);
static void _Transceiver_cbTxDone(uint32 status);
static void _Transceiver_cbPreDet(uint32 status);
static void _Transceiver_cbRxTo(uint32 status);
static void _Transceiver_cbPreTo(uint32 status);
// For handling MC sleep mode
static void MC_GoToSleep(void);
static void _Transceiver_PeriphConfig(void);
static void SYSCLKConfig_STOP(void);



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

	
	// Config controllers periptherals
	_Transceiver_PeriphConfig();
	
	do {
		sleep_10ms(100);
		reset_DW1000();	
		spi_set_rate_low();
	} while ( dwt_initialise(DWT_LOADUCODE) == DWT_ERROR );
    
	spi_set_rate_high();    
    dwt_configure(&config);	
	
	dwt_setcallbacks(
		&_Transceiver_cbRxOk,
		&_Transceiver_cbTxDone,		
		&_Transceiver_cbRxTo,
		&_Transceiver_cbPreDet,
		&_Transceiver_cbPreTo
	);
	
	// Sets IRQ handler
	port_set_deca_isr(dwt_isr);	
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
	uint32 IRQ_MASK;
	uint8 flags = DWT_START_TX_IMMEDIATE; // 0
	
	// Disable current events
	dwt_setinterrupt(SYS_MASK_MASK_32, 0);
	if (config->rx_aftertx_delay) // Response expected -> Enable transceiver events - receive, rx TO		
		IRQ_MASK = SYS_MASK_MRXFCG | SYS_MASK_MRXRFTO;	
	else // Enable transceiver events - sent, receive, rx TO		
		IRQ_MASK = SYS_MASK_MTXFRS; 
	dwt_setinterrupt(IRQ_MASK, 1);
	
	cbRes = 0;
	
	if (config->rx_aftertx_delay) {
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
	
	MC_GoToSleep();
	
	if (config->rx_aftertx_delay) { // Response expected
		dwt_receiverautoreenabled(1);
		
		while ( !(cbRes & (Transceiver_RXFCG | Transceiver_RXRFTO)) ) 
		{
			;			
		}
		
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
		{
			;
		}
	}	
	
	return cbRes;
}



Transceiver_RESULT Transceiver_Receive(Transceiver_RxConfig *config)
{
	uint32 frame_len;
	uint16 rxTO_ms, rxTO_us;
	
	// Disable current events
	dwt_setinterrupt(SYS_MASK_MASK_32, 0);
	// Enable transceiver events - receive, rx TO
	dwt_setinterrupt(SYS_MASK_MRXFCG | SYS_MASK_MRXRFTO, 1);
	
	cbRes = 0;
	
	dwt_receiverautoreenabled(1);	
	
	if (config->rx_timeout < 0xFFFFUL) {
		dwt_setrxtimeout(config->rx_timeout);	
	} else { // internal timer to be used
		dwt_setrxtimeout(0);
		rxTO_ms = (config->rx_timeout + config->rx_delay) / 1000;
		rxTO_us = (config->rx_timeout + config->rx_delay) % 1000;
		BaseTimer_SetPeriod(rxTO_ms);
		BaseTimer_Reset();
		BaseTimer_Enable();			
	}
	
	if (config->rx_delay) {		
		dwt_setdelayedtrxtime(config->rx_delay);
		dwt_rxenable(DWT_START_RX_DELAYED);		
	} else {
		dwt_setdelayedtrxtime(0);
		dwt_rxenable(DWT_START_RX_IMMEDIATE);
	}
	
	MC_GoToSleep();	
		
	// Waiting message or timeout	
	while ( !(cbRes & (Transceiver_RXFCG | Transceiver_RXRFTO)) ) {		
		if (config->rx_interrupt != NULL) { // interrupts the process if needed
			if (config->rx_interrupt() != 0) {
				_Transceiver_rxoff();
				return Transceiver_INTERRUPTED;
			}
		}
		if (BaseTimer_GetState() == BaseTimer_SET) { // if rx timeout >= 65535 us -> internal MC timer is used
			BaseTimer_Disable();
			BaseTimer_Reset();
			_Transceiver_rxoff();			
			if (rxTO_us > 0) {
				dwt_setrxtimeout(rxTO_us);
				dwt_rxenable(DWT_START_RX_IMMEDIATE);
			} else {
				cbRes = Transceiver_RXRFTO;
				break;
			}			
		}
	}		
	

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
	// Disable current events
	dwt_setinterrupt(SYS_MASK_MASK_32, 0);
	// Enable preamble events
	dwt_setinterrupt(SYS_MASK_MRXPRD | SYS_MASK_MRXPTO, 1);
	
	cbRes = 0;
	
	dwt_setpreambledetecttimeout(timeout);
	dwt_rxenable(DWT_START_RX_IMMEDIATE);
	
	MC_GoToSleep();	

	while ( !(cbRes & (Transceiver_RXPRD | Transceiver_RXPTO)) )
		;
		
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
	
	dwt_setrxtimeout(0);
	dwt_setdelayedtrxtime(0);
	dwt_receiverautoreenabled(1);
	dwt_rxenable(DWT_START_RX_IMMEDIATE);
}



inline void Transceiver_ReceiverOff(void)
{
	_Transceiver_rxoff();
}


		
inline Transceiver_RESULT Transceiver_GetReceptionResult(void)
{
	return cbRes;
}
	
	
	
uint8 Transceiver_GetAvailableData(uint8 *buffer)
{
	if ( cbRes == Transceiver_RXFCG ) {
		cbRes = 0; // reset
		
		uint32 frame_len = dwt_read32bitreg(RX_FINFO_ID) & RX_FINFO_RXFL_MASK_1023;
		dwt_readrxdata(buffer, frame_len, 0);
		return (uint8)frame_len;
	}
	return 0;
}



static void _Transceiver_cbRxOk(uint32 status)
{
	cbRes = Transceiver_RXFCG;
	_Transceiver_rxoff();
}


static void _Transceiver_cbTxDone(uint32 status)
{
	cbRes = Transceiver_TXFRS;
}


static void _Transceiver_cbPreDet(uint32 status)
{
	cbRes = Transceiver_RXPRD;
	_Transceiver_rxoff();
	
}


static void _Transceiver_cbRxTo(uint32 status)
{
	cbRes = Transceiver_RXRFTO;
	_Transceiver_rxoff();
}


static void _Transceiver_cbPreTo(uint32 status)
{
	cbRes = Transceiver_RXPTO;
	_Transceiver_rxoff();
}

				

static void MC_GoToSleep(void)
{
	// Request to enter STOP mode with regulator in low power mode
    //PWR_EnterSTOPMode(PWR_Regulator_LowPower, PWR_STOPEntry_WFI);	
	// Configures system clock after wake-up from STOP
    //SYSCLKConfig_STOP();
}
				


static void _Transceiver_PeriphConfig(void)
{
	// Initialises RCC, SPI, EXTI
	peripherals_init();  
	BaseTimer_SetPrescaler(SystemCoreClock / 1000); // ms
	BaseTimer_Reset();
	
	// 
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE);
}



static void SYSCLKConfig_STOP(void)
{  
	// After wake-up from STOP reconfigure the system clock
	// Enable HSE
	RCC_HSEConfig(RCC_HSE_ON);

	// Wait till HSE is ready
	while (RCC_GetFlagStatus(RCC_FLAG_HSERDY) == RESET)
		;

	// Enable PLL
	RCC_PLLCmd(ENABLE);

	// Wait till PLL is ready
	while (RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		;

	// Select PLL as system clock source
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

	// Wait till PLL is used as system clock source 
	while (RCC_GetSYSCLKSource() != 0x08)
		;
}


