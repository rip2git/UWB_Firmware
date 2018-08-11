#ifndef mGENERALTIMER_H
#define mGENERALTIMER_H

#include <stdint.h>
#include "stm32f0xx.h"


/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: EventTimer_STATE
 *
 * @brief: Used for determine the state of the event provided by timer
 *
*/
typedef enum {
	EventTimer_RESET = 0,
	EventTimer_SET = (!EventTimer_RESET)
} EventTimer_STATE;

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: EventTimer_IRQHandler
 *
 * @brief: Internal IRQHandler name
 *
*/
#define EventTIM2_IRQHandler(x) 	TIM2_IRQHandler(x)
#define EventTIM6_IRQHandler(x) 	TIM6_IRQHandler(x)
#define EventTIM14_IRQHandler(x) 	TIM14_IRQHandler(x)


/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: EventTimer_Initialization
 *
 * @brief: Initializes peripherals and sets the timer state
 *
 * NOTE: 
 *
 * input parameters
 * @param TIM - TIM_TypeDef timer (general/base).
 * @param priority - NVIC_IRQChannelPriority.
 *
 * output parameters
 *
 * no return value
*/
extern void EventTimer_Initialization(TIM_TypeDef *TIM, uint8_t priority);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: EventTimer_SetPrescaler
 *
 * @brief: Sets a prescaler of the timer
 *
 * NOTE: 
 *
 * input parameters
 * @param TIM - TIM_TypeDef timer (general/base).
 * @param prescaler - count of proceesors ticks before a part of the event will be reached.
 * The number of parts is determined by the period (described below)
 *
 * output parameters
 *
 * no return value
*/
extern void EventTimer_SetPrescaler(TIM_TypeDef *TIM, uint16_t prescaler);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: EventTimer_SetPeriod
 *
 * @brief: Sets a period of the timer
 *
 * NOTE: 
 *
 * input parameters
 * @param TIM - TIM_TypeDef timer (general/base).
 * @param prescaler - number of timers ticks before the event will be reached
 *
 * output parameters
 *
 * no return value
*/
extern void EventTimer_SetPeriod(TIM_TypeDef *TIM, uint16_t period);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: EventTimer_Enable
 *
 * @brief: Enable timer
 *
 * NOTE: 
 *
 * input parameters
 * @param TIM - TIM_TypeDef timer (general/base).
 *
 * output parameters
 *
 * no return value
*/
extern void EventTimer_Enable(TIM_TypeDef *TIM);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: EventTimer_Disable
 *
 * @brief: Disable timer
 *
 * NOTE: 
 *
 * input parameters
 * @param TIM - TIM_TypeDef timer (general/base).
 *
 * output parameters
 *
 * no return value
*/
extern void EventTimer_Disable(TIM_TypeDef *TIM);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: EventTimer_Set
 *
 * @brief: Sets the current timers ticks
 *
 * NOTE: 
 *
 * input parameters
 * @param TIM - TIM_TypeDef timer (general/base).
 * @param cnt - new current timers ticks.
 *
 * output parameters
 *
 * no return value
*/
extern void EventTimer_Set(TIM_TypeDef *TIM, uint16_t cnt);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: EventTimer_SetEvent
 *
 * @brief: Sets event up
 *
 * NOTE:
 *
 * input parameters
 * @param TIM - TIM_TypeDef timer (general/base).
 *
 * output parameters
 *
 * no return value
*/
extern void EventTimer_SetEvent(TIM_TypeDef *TIM);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: EventTimer_Get
 *
 * @brief: Get the current timers ticks
 *
 * NOTE: 
 *
 * input parameters
 * @param TIM - TIM_TypeDef timer (general/base).
 *
 * output parameters
 *
 * return value is the timers ticks
*/
extern uint16_t EventTimer_Get(TIM_TypeDef *TIM);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: EventTimer_GetState
 *
 * @brief: Used to determine the state of the event
 *
 * NOTE: 
 *
 * input parameters
 * @param TIM - TIM_TypeDef timer (general/base).
 *
 * output parameters
 *
 * return value is the state of the event
*/
extern EventTimer_STATE EventTimer_GetState(TIM_TypeDef *TIM);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: EventTimer_Reset
 *
 * @brief: Resets the event and the current timer ticks
 *
 * NOTE: 
 *
 * input parameters
 * @param TIM - TIM_TypeDef timer (general/base).
 *
 * output parameters
 *
 * no return value
*/
extern void EventTimer_Reset(TIM_TypeDef *TIM);


#endif
