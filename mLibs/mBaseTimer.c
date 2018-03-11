#include "mBaseTimer.h"
#include "stm32f0xx.h"


#define mTIMx		TIM6
#define mTIMx_CLK	RCC_APB1Periph_TIM6
#define mTIMx_IRQ	TIM6_DAC_IRQn



volatile uint8_t BaseTimer_Event = 0;



void BaseTimer_Initialization(void)
{
	TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(mTIMx_CLK, ENABLE);	
	
	NVIC_InitStructure.NVIC_IRQChannel = mTIMx_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_TimeBaseStructInit( &TIM_TimeBaseStructure );
	TIM_TimeBaseStructure.TIM_Prescaler = SystemCoreClock / 1000;
	TIM_TimeBaseStructure.TIM_Period = 500;	
	TIM_TimeBaseInit(mTIMx, &TIM_TimeBaseStructure);	
	
	TIM_ITConfig(mTIMx, TIM_IT_Update, ENABLE);	// by overrun
	
	BaseTimer_Disable();
}



inline BaseTimer_STATE BaseTimer_GetState(void)
{
	return (BaseTimer_Event == 0)? BaseTimer_RESET : BaseTimer_SET;
		
}



inline void BaseTimer_SetPrescaler(uint16_t prescaler)
{
	mTIMx->PSC = prescaler - 1; 
  	mTIMx->EGR = TIM_PSCReloadMode_Immediate;
}



inline void BaseTimer_SetPeriod(uint16_t period)
{
	mTIMx->ARR = period;
}



inline void BaseTimer_Enable(void)
{
	mTIMx->CR1 |= TIM_CR1_CEN;
}



inline void BaseTimer_Disable(void)
{
	mTIMx->CR1 &= (uint16_t)(~((uint16_t)TIM_CR1_CEN));
}


inline void BaseTimer_Reset(void)
{
	BaseTimer_Event = 0;
	mTIMx->CNT = 0;
}



inline void BaseTimer_Set(uint16_t cnt)
{
	mTIMx->CNT = cnt;
}


inline uint16_t BaseTimer_Get(void)
{
	return mTIMx->CNT;
}