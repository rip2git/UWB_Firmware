#ifndef mGENERALTIMER_H
#define mGENERALTIMER_H

#include <stdint.h>


/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: GeneralTimer_STATE
 *
 * @brief: Used for determine the state of the event provided by timer
 *
*/
typedef uint8_t GeneralTimer_STATE;

#define GeneralTimer_RESET		(0)
#define GeneralTimer_SET		(!GeneralTimer_RESET)

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: GeneralTimer_IRQHandler
 *
 * @brief: Internal IRQHandler name
 *
*/
#define GeneralTimer_IRQHandler(x) TIM14_IRQHandler(x)


/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: GeneralTimer_Initialization
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
extern void GeneralTimer_Initialization(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: GeneralTimer_SetPrescaler
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
extern inline void GeneralTimer_SetPrescaler(uint16_t prescaler);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: GeneralTimer_SetPeriod
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
extern inline void GeneralTimer_SetPeriod(uint16_t period);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: GeneralTimer_Enable
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
extern inline void GeneralTimer_Enable(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: GeneralTimer_Disable
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
extern inline void GeneralTimer_Disable(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: GeneralTimer_Set
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
extern inline void GeneralTimer_Set(uint16_t cnt);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: GeneralTimer_Get
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
extern inline uint16_t GeneralTimer_Get(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: GeneralTimer_GetState
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
extern GeneralTimer_STATE GeneralTimer_GetState(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: GeneralTimer_Reset
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
extern void GeneralTimer_Reset(void);


#endif