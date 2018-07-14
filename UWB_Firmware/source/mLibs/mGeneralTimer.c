#include "mGeneralTimer.h"
#include "stm32f0xx.h"



static volatile uint8_t GeneralTIM2_Event = 0;
static volatile uint8_t GeneralTIM14_Event = 0;



void GeneralTIM2_IRQHandler(void)
{
	if ( TIM2->SR & TIM_IT_Update )
	{
		TIM2->SR = (uint16_t)~TIM_IT_Update;
		GeneralTIM2_Event++;
	}
}



void GeneralTIM14_IRQHandler(void)
{
	if ( TIM14->SR & TIM_IT_Update )
	{
		TIM14->SR = (uint16_t)~TIM_IT_Update;
		GeneralTIM14_Event++;
	}
}



void GeneralTimer_Initialization(TIM_TypeDef *TIM)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	if (TIM == TIM2) {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
		NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	} else if (TIM == TIM14) {
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM14, ENABLE);
		NVIC_InitStructure.NVIC_IRQChannel = TIM14_IRQn;
	} else {
		return;
	}
	
	NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_TimeBaseStructInit( &TIM_TimeBaseStructure );
	TIM_TimeBaseStructure.TIM_Prescaler = SystemCoreClock / 1000;
	TIM_TimeBaseStructure.TIM_Period = 500;	
	TIM_TimeBaseInit(TIM, &TIM_TimeBaseStructure);
	
	TIM_ITConfig(TIM, TIM_IT_Update, ENABLE);	// by overrun
	
	GeneralTimer_Disable(TIM);
}



GeneralTimer_STATE GeneralTimer_GetState(TIM_TypeDef *TIM)
{
	GeneralTimer_STATE res;
	if (TIM == TIM2) {
		res = (GeneralTIM2_Event == 0)? GeneralTimer_RESET : GeneralTimer_SET;
	} else if (TIM == TIM14) {
		res = (GeneralTIM14_Event == 0)? GeneralTimer_RESET : GeneralTimer_SET;
	} else {
		res = GeneralTimer_RESET;
	}
	return res;
}



void GeneralTimer_SetPrescaler(TIM_TypeDef *TIM, uint16_t prescaler)
{
	TIM->PSC = prescaler - 1;
  	TIM->EGR = TIM_PSCReloadMode_Immediate;
}



void GeneralTimer_SetPeriod(TIM_TypeDef *TIM, uint16_t period)
{
	TIM->ARR = period;
}



void GeneralTimer_Enable(TIM_TypeDef *TIM)
{
	TIM->CR1 |= (uint16_t)TIM_CR1_CEN;
}



void GeneralTimer_Disable(TIM_TypeDef *TIM)
{
	TIM->CR1 &= (uint16_t)~TIM_CR1_CEN;
}



void GeneralTimer_Reset(TIM_TypeDef *TIM)
{
	if (TIM == TIM2) {
		GeneralTIM2_Event = 0;
	} else if (TIM == TIM14) {
		GeneralTIM14_Event = 0;
	} else {
		return;
	}
	TIM->CNT = 0;
}



void GeneralTimer_Set(TIM_TypeDef *TIM, uint16_t cnt)
{
	TIM->CNT = cnt;
}



void GeneralTimer_SetEvent(TIM_TypeDef *TIM)
{
	if (TIM == TIM2) {
		GeneralTIM2_Event = 1;
	} else if (TIM == TIM14) {
		GeneralTIM14_Event = 1;
	} else {
		return;
	}
}



uint16_t GeneralTimer_Get(TIM_TypeDef *TIM)
{
	return TIM->CNT;
}
