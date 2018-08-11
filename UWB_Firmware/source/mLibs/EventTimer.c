#include <EventTimer.h>
#include "stm32f0xx.h"


static volatile uint8_t EventTIM2_Event = 0;
static volatile uint8_t EventTIM6_Event = 0;
static volatile uint8_t EventTIM14_Event = 0;



void EventTIM2_IRQHandler(void)
{
	if ( TIM2->SR & TIM_IT_Update )
	{
		TIM2->SR = (uint16_t)~TIM_IT_Update;
		EventTIM2_Event++;
	}
}



void EventTIM6_IRQHandler(void)
{
	if ( TIM6->SR & TIM_IT_Update )
	{
		TIM6->SR = (uint16_t)~TIM_IT_Update;
		EventTIM6_Event++;
	}
}



void EventTIM14_IRQHandler(void)
{
	if ( TIM14->SR & TIM_IT_Update )
	{
		TIM14->SR = (uint16_t)~TIM_IT_Update;
		EventTIM14_Event++;
	}
}



void EventTimer_Initialization(TIM_TypeDef *TIM, uint8_t priority)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	if (TIM == TIM2) {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
		NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	} else if (TIM == TIM6) {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);
		NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn;
	} else if (TIM == TIM14) {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
		NVIC_InitStructure.NVIC_IRQChannel = TIM14_IRQn;
	} else {
		return;
	}
	
	NVIC_InitStructure.NVIC_IRQChannelPriority = priority;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_TimeBaseStructInit( &TIM_TimeBaseStructure );
	TIM_TimeBaseStructure.TIM_Prescaler = SystemCoreClock / 1000;
	TIM_TimeBaseStructure.TIM_Period = 500;	
	TIM_TimeBaseInit(TIM, &TIM_TimeBaseStructure);
	
	TIM_ITConfig(TIM, TIM_IT_Update, ENABLE);	// by overrun
	
	EventTimer_Disable(TIM);
}



EventTimer_STATE EventTimer_GetState(TIM_TypeDef *TIM)
{
	EventTimer_STATE res;
	if (TIM == TIM2) {
		res = (EventTIM2_Event == 0)? EventTimer_RESET : EventTimer_SET;
	} else if (TIM == TIM6) {
		res = (EventTIM6_Event == 0)? EventTimer_RESET : EventTimer_SET;
	} else if (TIM == TIM14) {
		res = (EventTIM14_Event == 0)? EventTimer_RESET : EventTimer_SET;
	} else {
		res = EventTimer_RESET;
	}
	return res;
}



void EventTimer_SetPrescaler(TIM_TypeDef *TIM, uint16_t prescaler)
{
	TIM->PSC = prescaler - 1;
  	TIM->EGR = TIM_PSCReloadMode_Immediate;
}



void EventTimer_SetPeriod(TIM_TypeDef *TIM, uint16_t period)
{
	TIM->ARR = period;
}



void EventTimer_Enable(TIM_TypeDef *TIM)
{
	TIM->CR1 |= (uint16_t)TIM_CR1_CEN;
}



void EventTimer_Disable(TIM_TypeDef *TIM)
{
	TIM->CR1 &= (uint16_t)~TIM_CR1_CEN;
}



void EventTimer_Reset(TIM_TypeDef *TIM)
{
	if (TIM == TIM2) {
		EventTIM2_Event = 0;
	} else if (TIM == TIM6) {
		EventTIM6_Event = 0;
	} else if (TIM == TIM14) {
		EventTIM14_Event = 0;
	} else {
		return;
	}
	TIM->CNT = 0;
}



void EventTimer_Set(TIM_TypeDef *TIM, uint16_t cnt)
{
	TIM->CNT = cnt;
}



void EventTimer_SetEvent(TIM_TypeDef *TIM)
{
	if (TIM == TIM2) {
		EventTIM2_Event = 1;
	} else if (TIM == TIM6) {
		EventTIM6_Event = 1;
	} else if (TIM == TIM14) {
		EventTIM14_Event = 1;
	} else {
		return;
	}
}



uint16_t EventTimer_Get(TIM_TypeDef *TIM)
{
	return TIM->CNT;
}
