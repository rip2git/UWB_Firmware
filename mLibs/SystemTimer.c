#include "SystemTimer.h"
#include "stm32f0xx.h"


static uint32_t ticks;
static uint8_t event;



void SysTick_Handler(void)
{
	event++;
}



void SystemTimer_Initialization(void)
{
	event = 0;	
	ticks = SystemCoreClock / 1000; // ms	
	NVIC_SetPriority(SysTick_IRQn, (1<<__NVIC_PRIO_BITS) - 1);
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}



void SystemTimer_Sleep(uint32_t time_ms)
{  
	event = 0;
	SysTick->LOAD  = (ticks * time_ms) - 1;
	SysTick->VAL   = 0;
	SysTick->CTRL  = 	SysTick_CTRL_CLKSOURCE_Msk |
                   		SysTick_CTRL_TICKINT_Msk   |
                   		SysTick_CTRL_ENABLE_Msk;
  
	/* TODO: should add MC sleep here */
	while ( !event )
		;
  
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
}
