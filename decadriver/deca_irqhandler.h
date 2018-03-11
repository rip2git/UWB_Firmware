#ifndef DECA_IRQHANDLER_
#define DECA_IRQHANDLER_


// Using standart decawave handlers deca_isr (implemented deca_device.c)
//#define DECA_STD_HANDLERS


#ifndef DECA_STD_HANDLERS


#include "deca_types.h"


// Call-back type for all events
typedef void (*dwt_cb_t)(uint32);


/*! ------------------------------------------------------------------------------------------------------------------
 * @fn dwt_setcallbacks()
 *
 * @brief This function is used to register the different callbacks called when one of the corresponding event occurs.
 *
 * NOTE: Callbacks can be undefined (set to NULL). In this case, dwt_isr() will process the event as usual but the 'null'
 * callback will not be called.
 *
 * input parameters
 * @param cbRxOk - the pointer to the RX good frame event callback function
 * @param cbTxDone - the pointer to the TX confirmation event callback function
 * @param cbRxTo - the pointer to the RX timeout events callback function
 * @param cbPreDet - the pointer to the preamble detect event callback function
 * @param cbPreTo - the pointer to the preamble timeout event callback function
 *
 * output parameters
 *
 * no return value
 */
extern void dwt_setcallbacks(
	dwt_cb_t cbRxOk, 
	dwt_cb_t cbTxDone, 	
	dwt_cb_t cbRxTo, 
	dwt_cb_t cbPreDet, 
	dwt_cb_t cbPreTo
);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn dwt_checkirq()
 *
 * @brief This function checks if the IRQ line is active - this is used instead of interrupt handler
 *
 * input parameters
 *
 * output parameters
 *
 * return value is 1 if the IRQS bit is set and 0 otherwise
 */
extern uint8 dwt_checkirq(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn dwt_isr()
 *
 * @brief This is the DW1000's general Interrupt Service Routine. It will process/report the following events:
 *          - RXFCG (through cbRxOk callback)
 *          - TXFRS (through cbTxDone callback)
 *          - RXRFTO (through cbRxTo callback)
 *			- RXPRD (through cbPreDet callback)
 *			- RXPTO (hrough cbPreTo callback)
 *        For all events, corresponding interrupts are cleared and necessary resets are performed.
 *
 * NOTE: There is expected 1 interrupt, so others would be ignored. Priority of expected interrupts is presented 
 * in brief in direct order
 *
 *        /!\ This version of the ISR does not supports double buffering and automatic RX re-enabling!
 *
 * input parameters
 *
 * output parameters
 *
 * no return value
 */
extern void dwt_isr(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn void dwt_setinterrupt()
 *
 * @brief This function enables the specified events to trigger an interrupt.
 * The following events can be enabled:
 * 		SYS_MASK_ID - MASKs register
 * 		SYS_STATUS_ID - Events register
 *
 * input parameters:
 * @param bitmask - sets the events which will generate interrupt
 * @param enable - if set the interrupts are enabled else they are cleared
 *
 * output parameters
 *
 * no return value
 */
extern void dwt_setinterrupt(uint32 bitmask, uint8 enable);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn dwt_isr_lplisten()
 *
 * @brief This is the DW1000's Interrupt Service Routine to use when low-power listening scheme is implemented. It will
 *        only process/report the RXFCG event (through cbRxOk callback).
 *        It clears RXFCG interrupt and reads received frame information and frame control before calling the callback.
 *
 *        /!\ This version of the ISR is designed for single buffering case only!
 *
 * input parameters
 *
 * output parameters
 *
 * no return value
 */
//extern void dwt_lowpowerlistenisr(void);



#endif // DECA_STD_HANDLERS


#endif