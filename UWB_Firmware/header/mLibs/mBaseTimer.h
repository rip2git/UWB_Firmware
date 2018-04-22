#ifndef mBASETIMER_H
#define mBASETIMER_H

#include <stdint.h>


/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: BaseTimer_STATE
 *
 * @brief: Used for determine the state of the event provided by timer
 *
*/
typedef enum {
	BaseTimer_RESET = 0,
	BaseTimer_SET = (!BaseTimer_RESET)
} BaseTimer_STATE;

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: BaseTimer_IRQHandler
 *
 * @brief: Internal IRQHandler name
 *
*/
#define BaseTimer_IRQHandler(x) TIM6_DAC_IRQHandler(x)


/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: BaseTimer_Initialization
 *
 * @brief: Initializes peripherals and sets the timer state
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * no return value
*/
extern void BaseTimer_Initialization(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: BaseTimer_SetPrescaler
 *
 * @brief: Sets a prescaler of the timer
 *
 * NOTE: 
 *
 * input parameters
 * @params prescaler - count of proceesors ticks before a part of the event will be reached. 
 * The number of parts is determined by the period (described below)
 *
 * output parameters
 *
 * no return value
*/
extern inline void BaseTimer_SetPrescaler(uint16_t prescaler);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: BaseTimer_SetPeriod
 *
 * @brief: Sets a period of the timer
 *
 * NOTE: 
 *
 * input parameters
 * @params prescaler - number of timers ticks before the event will be reached
 *
 * output parameters
 *
 * no return value
*/
extern inline void BaseTimer_SetPeriod(uint16_t period);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: BaseTimer_Enable
 *
 * @brief: Enable timer
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * no return value
*/
extern inline void BaseTimer_Enable(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: BaseTimer_Disable
 *
 * @brief: Disable timer
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * no return value
*/
extern inline void BaseTimer_Disable(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: BaseTimer_Set
 *
 * @brief: Sets the current timers ticks
 *
 * NOTE: 
 *
 * input parameters
 * @param cnt - new current timers ticks
 *
 * output parameters
 *
 * no return value
*/
extern inline void BaseTimer_Set(uint16_t cnt);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: BaseTimer_Get
 *
 * @brief: Get the current timers ticks
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * return value is the timers ticks
*/
extern inline uint16_t BaseTimer_Get(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: BaseTimer_GetState
 *
 * @brief: Used to determine the state of the event
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * return value is the state of the event
*/
extern BaseTimer_STATE BaseTimer_GetState(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: BaseTimer_Reset
 *
 * @brief: Resets the event and the current timer ticks
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * no return value
*/
extern void BaseTimer_Reset(void);


#endif
