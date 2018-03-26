#ifndef SYSTEMTIMER_H_
#define SYSTEMTIMER_H_

#include <stdint.h>


/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: SysTick_Handler
 *
 * @brief: IRQ handler that called when time of sleep is reached
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * no return value
*/
extern void SysTick_Handler(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: SystemTimer_Initialization
 *
 * @brief: Initializes internal entities (prescaler, interrupt)
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * no return value
*/
extern void SystemTimer_Initialization(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: SystemTimer_Sleep
 *
 * @brief: 
 *
 * NOTE: 
 *
 * input parameters
 * @param time_ms - time to sleep in ms
 *
 * output parameters
 *
 * no return value
*/
extern void SystemTimer_Sleep(uint32_t time_ms);


#endif