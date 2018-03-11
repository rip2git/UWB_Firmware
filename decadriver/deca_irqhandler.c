
#include "deca_param_types.h"
#include "deca_regs.h"
#include "deca_device_api.h"



#ifndef DECA_STD_HANDLERS


static volatile uint32 tmp_vola = 0;



typedef struct
{
    uint32 		status;      		// initial value of register as ISR is entered
	dwt_cb_t    cbRxOk;             // Callback for RX good frame event    
	dwt_cb_t    cbTxDone;           // Callback for TX confirmation event    
    dwt_cb_t    cbRxTo;             // Callback for RX timeout events
	dwt_cb_t    cbPreDet;           // Callback for preamble detect event
	dwt_cb_t 	cbPreTo;			// Callback for preamble timeout event
} dwt_callbacks_t;



static dwt_callbacks_t _dw1000_callbacks;



void dwt_isr(void)
{
    _dw1000_callbacks.status = dwt_read32bitreg(SYS_STATUS_ID); // Read status register low 32bits
	
	//uint32 status_m = dwt_read32bitreg(SYS_MASK_ID) ;
	//tmp_vola += status_m;
	
    // Handle RX good frame event
    if(_dw1000_callbacks.status & SYS_STATUS_RXFCG)
    {
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_GOOD); // Clear all receive status bits    
		
        // Call the corresponding callback if present
        if(_dw1000_callbacks.cbRxOk != NULL) {
            _dw1000_callbacks.cbRxOk(_dw1000_callbacks.status);
        }
		return;
    }

    // Handle TX confirmation event
    if(_dw1000_callbacks.status & SYS_STATUS_TXFRS)
    {
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_TX); // Clear TX event bits

        // Call the corresponding callback if present
        if(_dw1000_callbacks.cbTxDone != NULL) {
            _dw1000_callbacks.cbTxDone(_dw1000_callbacks.status);
        }
		return;
    }
	
	// Handle frame reception timeout events
    if(_dw1000_callbacks.status & SYS_STATUS_RXRFTO)
    {
		// Clear RX error/timeout events in the DW1000 status register.
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXRFTO | SYS_STATUS_ALL_RX_ERR);
        
        // Call the corresponding callback if present
        if(_dw1000_callbacks.cbRxTo != NULL) {
            _dw1000_callbacks.cbRxTo(_dw1000_callbacks.status);
        }
		return;
    }
	
	// Handle preamble detection
	if (_dw1000_callbacks.status & SYS_STATUS_RXPRD)
	{
		dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXPRD);
			
		// Call the corresponding callback if present
        if(_dw1000_callbacks.cbPreDet != NULL) {
            _dw1000_callbacks.cbPreDet(_dw1000_callbacks.status);
        }
		return;
	}
	
	// Handle preamble timeout
	if (_dw1000_callbacks.status & SYS_STATUS_RXPTO)
	{
		dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_RXPTO);
		
		// Call the corresponding callback if present
        if(_dw1000_callbacks.cbPreTo != NULL) {
            _dw1000_callbacks.cbPreTo(_dw1000_callbacks.status);
        }
		return;
	}
}



void dwt_setinterrupt(uint32 bitmask, uint8 enable)
{
    decaIrqStatus_t stat ;
    uint32 mask ;

    // Need to beware of interrupts occurring in the middle of following read modify write cycle
    stat = decamutexon() ;

    mask = dwt_read32bitreg(SYS_MASK_ID) ; // Read register

    if(enable)
    {
        mask |= bitmask ;
    }
    else
    {
        mask &= ~bitmask ; // Clear the bit
    }
    dwt_write32bitreg(SYS_MASK_ID, mask) ; // New value

    decamutexoff(stat) ;
}



void dwt_setcallbacks(
	dwt_cb_t cbRxOk, 
	dwt_cb_t cbTxDone,	
	dwt_cb_t cbRxTo, 
	dwt_cb_t cbPreDet, 
	dwt_cb_t cbPreTo
)
{    
    _dw1000_callbacks.cbRxOk = cbRxOk;
	_dw1000_callbacks.cbTxDone = cbTxDone;	
    _dw1000_callbacks.cbRxTo = cbRxTo;
	_dw1000_callbacks.cbPreDet = cbPreDet;
	_dw1000_callbacks.cbPreTo = cbPreTo;
}



uint8 dwt_checkirq(void)
{
    // Reading the lower byte only is enough for this operation
	return (dwt_read8bitoffsetreg(SYS_STATUS_ID, SYS_STATUS_OFFSET) & SYS_STATUS_IRQS); 
}



// wasn't implemented
void dwt_lowpowerlistenisr(void)
{
    /*uint32 status = pdw1000local->cbData.status = dwt_read32bitreg(SYS_STATUS_ID); // Read status register low 32bits
    uint16 finfo16;
    uint16 len;

    // The only interrupt handled when in low-power listening mode is RX good frame so proceed directly to the handling of the received frame.

    // Deactivate low-power listening before clearing the interrupt. If not, the DW1000 will go back to sleep as soon as the interrupt is cleared.
    dwt_setlowpowerlistening(0);

    dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_ALL_RX_GOOD); // Clear all receive status bits

    pdw1000local->cbData.rx_flags = 0;

    // Read frame info - Only the first two bytes of the register are used here.
    finfo16 = dwt_read16bitoffsetreg(RX_FINFO_ID, 0);

    // Report frame length - Standard frame length up to 127, extended frame length up to 1023 bytes
    len = finfo16 & RX_FINFO_RXFL_MASK_1023;
    if(pdw1000local->longFrames == 0)
    {
        len &= RX_FINFO_RXFLEN_MASK;
    }
    pdw1000local->cbData.datalength = len;

    // Report ranging bit
    if(finfo16 & RX_FINFO_RNG)
    {
        pdw1000local->cbData.rx_flags |= DWT_CB_DATA_RX_FLAG_RNG;
    }

    // Report frame control - First bytes of the received frame.
    dwt_readfromdevice(RX_BUFFER_ID, 0, FCTRL_LEN_MAX, pdw1000local->cbData.fctrl);

    // Because of a previous frame not being received properly, AAT bit can be set upon the proper reception of a frame not requesting for
    // acknowledgement (ACK frame is not actually sent though). If the AAT bit is set, check ACK request bit in frame control to confirm (this
    // implementation works only for IEEE802.15.4-2011 compliant frames).
    // This issue is not documented at the time of writing this code. It should be in next release of DW1000 User Manual (v2.09, from July 2016).
    if((status & SYS_STATUS_AAT) && ((pdw1000local->cbData.fctrl[0] & FCTRL_ACK_REQ_MASK) == 0))
    {
        dwt_write32bitreg(SYS_STATUS_ID, SYS_STATUS_AAT); // Clear AAT status bit in register
        pdw1000local->cbData.status &= ~SYS_STATUS_AAT; // Clear AAT status bit in callback data register copy
        pdw1000local->wait4resp = 0;
    }

    // Call the corresponding callback if present
    if(pdw1000local->cbRxOk != NULL)
    {
        pdw1000local->cbRxOk(&pdw1000local->cbData);
    }*/
}
	


#endif // DECA_STD_HANDLERS
