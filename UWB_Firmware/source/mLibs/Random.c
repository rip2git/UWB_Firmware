#include "Random.h"

#include "stm32f0xx.h"
#include <stdlib.h>



#define RND_PIN			GPIO_Pin_1 	// ADC_IN9
#define RND_GPIO		GPIOB
#define RND_CLK			RCC_AHBPeriph_GPIOB
#define RND_ADC_CHNL	ADC_Channel_9



void Random_Initialization(void)
{
	ADC_InitTypeDef ADC_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	// clock cfg
	RCC_AHBPeriphClockCmd(RND_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	
	// pin cfg
	GPIO_InitStructure.GPIO_Pin = RND_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(RND_GPIO, &GPIO_InitStructure);
	
	// ADC cfg
	ADC_DeInit(ADC1);
	ADC_StructInit(&ADC_InitStructure);
	ADC_InitStructure.ADC_Resolution = ADC_Resolution_12b;
	ADC_InitStructure.ADC_ContinuousConvMode = ENABLE; 
	ADC_InitStructure.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStructure.ADC_ScanDirection = ADC_ScanDirection_Upward;
	ADC_Init(ADC1, &ADC_InitStructure); 
	
	ADC_ChannelConfig(ADC1, RND_ADC_CHNL , ADC_SampleTime_239_5Cycles);	
	ADC_GetCalibrationFactor(ADC1);
	ADC_Cmd(ADC1, ENABLE);  
	
	while( !ADC_GetFlagStatus(ADC1, ADC_FLAG_ADRDY) )
		; 
  	ADC_StartOfConversion(ADC1);
	
	// start convertion
	while(ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC) == RESET)
		;
	
	// get value
	uint16_t ADC1ConvertedValue;
	ADC1ConvertedValue = ADC_GetConversionValue(ADC1);	
	
	// disable ADC & pin
	ADC_Cmd(ADC1, DISABLE); 
	while ( ADC_GetFlagStatus(ADC1, ADC_FLAG_ADDIS) )
		;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, DISABLE);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // tri-state
	GPIO_Init(RND_GPIO, &GPIO_InitStructure);	
	
	// set seed
	srand( ADC1ConvertedValue );	
}



unsigned long Random_GetNumber(unsigned long min, unsigned long max)
{
	unsigned long tmp = rand();
	return (tmp % max) + min;
}
