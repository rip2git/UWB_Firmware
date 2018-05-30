#include "mUSART_DMA.h"
#include <string.h>



#define USARTx					USART1                  //USART2
#define USARTx_GPIO				GPIOA
#define USARTx_TX 				GPIO_Pin_9              //GPIO_Pin_2      
#define USARTx_RX				GPIO_Pin_10             //GPIO_Pin_3 
#define USARTx_PinSource_TX 	GPIO_PinSource9         //GPIO_PinSource2      
#define USARTx_PinSource_RX 	GPIO_PinSource10        //GPIO_PinSource3  
#define USARTx_CLK				RCC_APB2Periph_USART1   //RCC_APB1Periph_USART2
#define USARTx_GPIO_CLK			RCC_AHBPeriph_GPIOA
#define USARTx_BaudRate			2000000 //2000000 //(115200)

#define DMAx					DMA1
#define DMAx_ChTX				DMA1_Channel4
#define DMAx_ChRX				DMA1_Channel5
#define DMAx_USARTx_ChTX		DMA1_CH4_USART1_TX
#define DMAx_USARTx_ChRX		DMA1_CH5_USART1_RX
#define DMAx_ChTX_Success_Fl	DMA_ISR_TCIF4
#define DMAx_ChTX_Fail_Fl		DMA_ISR_TEIF4
#define DMAx_ChTX_FLAG_MASK		DMA_IFCR_CGIF4
#define DMAx_ChRX_Success_Fl	DMA_ISR_TCIF5
#define DMAx_ChRX_Fail_Fl		DMA_ISR_TEIF5
#define DMAx_ChRX_FLAG_MASK		DMA_IFCR_CGIF5
#define DMAx_IRQ				DMA1_Channel4_5_IRQn



static const uint32_t USARTx_ERROR_FLAGS = USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_ORE | USART_FLAG_PE;
//static volatile uint16_t dummyTmp;



typedef struct {
	USART_CALL_BACK RX_Success_CallBack;
	USART_CALL_BACK RX_Fail_CallBack;
} USART_CALL_BACKS;



static uint8_t USARTx_TXBuffer[USARTx_BUFFER_SIZE];
static uint8_t USARTx_RXBuffer[USARTx_BUFFER_SIZE];
static uint16_t _USARTx_RequestedLength;
static uint8_t *_USARTx_RequestedBuffer;
static USART_MODE _USARTx_mode;



static USART_CALL_BACKS _USART_CallBack;
static USART_RESULT _USART_SendByDMA(uint16_t size);



USART_RESULT USART_StartRead(uint8_t *buffer, uint16_t length)
{
	if (_USARTx_mode) // not supported
		return USART_FAIL;

	_USARTx_RequestedBuffer = buffer;
	_USARTx_RequestedLength = length;

	DMAx_ChRX->CNDTR = (uint32_t)length;	
	DMAx_ChRX->CCR |= DMA_CCR_EN;
	return USART_SUCCESS;
}



inline void USART_ForceReadEnd(void)
{
	DMAx_ChRX->CCR &= ~DMA_CCR_EN;
}



inline USART_RESULT USART_ErrorControl(void)
{
	if (USARTx->ISR & USARTx_ERROR_FLAGS) {
		DMAx_ChRX->CCR &= ~DMA_CCR_EN;
		USARTx->ICR = USARTx_ERROR_FLAGS;
		return USART_FAIL;
	}
	return USART_SUCCESS;
}



void USART_IRQHandler(void)
{
	DMAx_ChRX->CCR &= ~DMA_CCR_EN;	
	
	// RECEIVE COMPLETE
	if (DMAx->ISR & DMAx_ChRX_Success_Fl) {
		DMAx->IFCR = DMAx_ChRX_FLAG_MASK;
		memcpy((void*)_USARTx_RequestedBuffer, &(USARTx_RXBuffer[0]), _USARTx_RequestedLength);
		_USART_CallBack.RX_Success_CallBack();		
	}
	// ERROR OCCURRE
	else if (DMAx->ISR & DMAx_ChRX_Fail_Fl) {
		DMAx->IFCR = DMAx_ChRX_FLAG_MASK;
		_USART_CallBack.RX_Fail_CallBack();		
	}
}



static USART_RESULT _USART_SendByDMA(uint16_t size)
{
	DMAx_ChTX->CCR &= ~DMA_CCR_EN; // reset
	DMAx_ChTX->CNDTR = size;	
	DMAx_ChTX->CCR |= DMA_CCR_EN;
	
	if ( _USARTx_mode ) {		
		while ( !(DMAx->ISR & (DMAx_ChTX_Success_Fl | DMAx_ChTX_Fail_Fl)) )
			;		
		uint32_t fl = (DMAx->ISR & DMAx_ChTX_Success_Fl);
		DMAx->IFCR = DMAx_ChTX_FLAG_MASK; // reset
		return fl? USART_SUCCESS : USART_FAIL;
	} else {
		return USART_SUCCESS;
	}
}



USART_RESULT USART_SendByte(const uint8_t c)
{
	memcpy((void*)USARTx_TXBuffer, &c, 1);
	return _USART_SendByDMA(1);
}



USART_RESULT USART_SendBuffer(const uint8_t *buffer, uint16_t length)
{
	memcpy((void*)USARTx_TXBuffer, buffer, length);
	return _USART_SendByDMA(length);
}



USART_RESULT USART_SendString(const char *string)
{	
	uint16_t i = 0;
	while (string[i] != '\0') {		
		i++;
	}
	memcpy((void*)USARTx_TXBuffer, string, i);
	return _USART_SendByDMA(i);
}



void USART_Initialization(void)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	DMA_InitTypeDef DMA_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	// RCC ENABLE ---------------------------------------------------
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	RCC_AHBPeriphClockCmd(USARTx_GPIO_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(USARTx_CLK, ENABLE);		
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	
	
	// REMAPING -----------------------------------------------------
	SYSCFG_DMAChannelRemapConfig(SYSCFG_DMARemap_USART1Rx, ENABLE);
	SYSCFG_DMAChannelRemapConfig(SYSCFG_DMARemap_USART1Tx, ENABLE);
	DMA_RemapConfig(DMAx, DMAx_USARTx_ChTX);
	DMA_RemapConfig(DMAx, DMAx_USARTx_ChRX); 	
	
	// GPIO CONFIG --------------------------------------------------   
    GPIO_PinAFConfig(USARTx_GPIO, USARTx_PinSource_TX, GPIO_AF_1);
	GPIO_PinAFConfig(USARTx_GPIO, USARTx_PinSource_RX, GPIO_AF_1);  
	//
	GPIO_InitStructure.GPIO_Pin = USARTx_TX | USARTx_RX;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_Init(USARTx_GPIO, &GPIO_InitStructure); 
	
	// USART CONFIG --------------------------------------------------
	USART_InitStructure.USART_BaudRate = USARTx_BaudRate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USARTx, &USART_InitStructure);
	
	// DMA CONFIG ---------------------------------------------------	
	DMA_InitStructure.DMA_BufferSize = 1;
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;	
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;
	//	
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)USARTx_TXBuffer;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Low;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (__IO uint32_t)&(USARTx->TDR);
	DMA_Init(DMAx_ChTX, &DMA_InitStructure);	
	//
	DMA_InitStructure.DMA_MemoryBaseAddr = (uint32_t)USARTx_RXBuffer;
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralSRC;
	DMA_InitStructure.DMA_Priority = DMA_Priority_Medium;
	DMA_InitStructure.DMA_PeripheralBaseAddr = (__IO uint32_t)&(USARTx->RDR);
	DMA_Init(DMAx_ChRX, &DMA_InitStructure);
	
	DMA_ITConfig(DMAx_ChRX, DMA_IT_TC | DMA_IT_TE, ENABLE);
	
	// NVIC CONFIG --------------------------------------------------
	NVIC_InitStructure.NVIC_IRQChannel = DMAx_IRQ;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_DMACmd(USARTx, USART_DMAReq_Tx | USART_DMAReq_Rx, ENABLE);
	USART_DMAReceptionErrorConfig(USARTx, USART_DMAOnError_Enable);
	USART_Cmd(USARTx, ENABLE);
	
	_USART_CallBack.RX_Success_CallBack = USART_NO_CallBack;
	_USART_CallBack.RX_Fail_CallBack = USART_NO_CallBack;
	
	_USARTx_mode = USART_NOWAIT;
	
	return;
}



void USART_SetTRXMode(USART_MODE mode) 
{
	_USARTx_mode = mode;
}



void USART_SetCallBacks(
	USART_CALL_BACK rx_success_cb,
	USART_CALL_BACK rx_fail_cb
) 
{
	_USART_CallBack.RX_Success_CallBack = rx_success_cb;
	_USART_CallBack.RX_Fail_CallBack = rx_fail_cb;
}


