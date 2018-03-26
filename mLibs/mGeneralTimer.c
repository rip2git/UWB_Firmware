#include "mGeneralTimer.h"
#include "stm32f0xx.h"


#define mTIMx		TIM14
#define mTIMx_CLK	RCC_APB1Periph_TIM14
#define mTIMx_IRQ	TIM14_IRQn



static volatile uint8_t GeneralTimer_Event = 0;



void GeneralTimer_IRQHandler(void)
{
	if ( mTIMx->SR & TIM_IT_Update )
	{
		mTIMx->SR = (uint16_t)~TIM_IT_Update;
		GeneralTimer_Event++;		
	}
}



void GeneralTimer_Initialization(void)
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
	
	GeneralTimer_Disable();
}



GeneralTimer_STATE GeneralTimer_GetState(void)
{
	return (GeneralTimer_Event == 0)? GeneralTimer_RESET : GeneralTimer_SET;
		
}



inline void GeneralTimer_SetPrescaler(uint16_t prescaler)
{
	mTIMx->PSC = prescaler - 1; 
  	mTIMx->EGR = TIM_PSCReloadMode_Immediate;
}



inline void GeneralTimer_SetPeriod(uint16_t period)
{
	mTIMx->ARR = period;
}



inline void GeneralTimer_Enable(void)
{
	mTIMx->CR1 |= (uint16_t)TIM_CR1_CEN;
}



inline void GeneralTimer_Disable(void)
{
	mTIMx->CR1 &= (uint16_t)~TIM_CR1_CEN;
}


void GeneralTimer_Reset(void)
{
	GeneralTimer_Event = 0;
	mTIMx->CNT = 0;
}



inline void GeneralTimer_Set(uint16_t cnt)
{
	mTIMx->CNT = cnt;
}


inline uint16_t GeneralTimer_Get(void)
{
	return mTIMx->CNT;
}