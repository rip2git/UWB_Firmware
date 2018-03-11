#include "mUSART.h"



#define USARTx					USART1                  //USART2
#define USARTx_GPIO				GPIOA
#define USARTx_TX 				GPIO_Pin_9              //GPIO_Pin_2      
#define USARTx_RX				GPIO_Pin_10             //GPIO_Pin_3 
#define USARTx_PinSource_TX 	GPIO_PinSource9         //GPIO_PinSource2      
#define USARTx_PinSource_RX 	GPIO_PinSource10        //GPIO_PinSource3  
#define USARTx_CLK				RCC_APB2Periph_USART1   //RCC_APB1Periph_USART2
#define USARTx_GPIO_CLK			RCC_AHBPeriph_GPIOA
#define USARTx_IRQn				USART1_IRQn             //USART2_IRQn
#define USARTx_BaudRate			(115200)



#define USARTx_RX_BUFFER_SIZE_def	255
#define USARTx_TX_BUFFER_SIZE_def	255


const uint16_t USARTx_RX_BUFFER_SIZE = USARTx_RX_BUFFER_SIZE_def;
const uint16_t USARTx_TX_BUFFER_SIZE = USARTx_TX_BUFFER_SIZE_def;

volatile uint8_t USARTx_ReceivingError;
//
volatile uint8_t USARTx_RxBuffer[USARTx_RX_BUFFER_SIZE_def];
volatile uint16_t USARTx_RxWr_Idx = 0, USARTx_RxRd_Idx = 0;
volatile uint16_t USARTx_RxCounter = 0;
//
volatile uint8_t USARTx_TxBuffer[USARTx_TX_BUFFER_SIZE_def];
volatile uint16_t USARTx_TxWr_Idx = 0, USARTx_TxRd_Idx = 0;
volatile uint16_t USARTx_TxCounter = 0;



inline uint16_t USART_GetRxStatus(void)
{
	return USARTx_RxCounter; // 0 -> empty
}



inline uint8_t USART_GetErrorStatus(void)
{
	return USARTx_ReceivingError;
}



inline void USART_ResetErrorStatus(void)
{
	USARTx_ReceivingError = 0;
}



uint8_t USART_GetByte(void)
{
	uint8_t data;
	
	while (USART_GetRxStatus() == USART_RX_EMPTY && USARTx_ReceivingError == 0)
		;
	
	if (USARTx_ReceivingError) 
		return 0;
	
	data = USARTx_RxBuffer[USARTx_RxRd_Idx++];
	
	if (USARTx_RxRd_Idx == USARTx_RX_BUFFER_SIZE) // кольцо буфера
		USARTx_RxRd_Idx = 0;
		
	USARTx->CR1 &= ~USART_CR1_RXNEIE; // rxne interrupt disable	
	//USART_ITConfig(USARTx, USART_IT_RXNE, DISABLE); 	// прер выкл
	--USARTx_RxCounter;
	USARTx->CR1 |= USART_CR1_RXNEIE; // rxne interrupt enable
	//USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);		// прер вкл
	
	return data;
}



void USART_SendByte(const uint8_t c)
{
	while (USARTx_TxCounter == USARTx_TX_BUFFER_SIZE) // буфер полон
		; 	
	USART1->CR1 &= ~USART_CR1_TXEIE; // txe interrupt disable
	//USART_ITConfig(USARTx, USART_IT_TXE, DISABLE); 	// прер выкл
	
	// в буфере уже что-то есть ИЛИ в данный момент что-то уже передается
	if (USARTx_TxCounter || !(USART1->ISR & USART_FLAG_TXE)) {
		USARTx_TxBuffer[USARTx_TxWr_Idx++] = c;
		++USARTx_TxCounter;
		if (USARTx_TxWr_Idx == USARTx_TX_BUFFER_SIZE) // кольцо буфера
			USARTx_TxWr_Idx = 0;			
		USARTx->CR1 |= USART_CR1_TXEIE; // txe interrupt enable
		//USART_ITConfig(USARTx, USART_IT_TXE, ENABLE);	// прер вкл
		
	} else {	// UART свободен
		USARTx->TDR = c;
	}
}



void USART_SendBuffer(const uint8_t *buffer, uint16_t length)
{
	uint16_t i;
	for (i = 0; i < length; ++i)
		USART_SendByte(buffer[i]);	
}



void USART_SendString(const char string[])
{
	uint16_t i = 0;
	while (string[i] != '\0') {
		USART_SendByte(string[i]);
		i++;
	}	
}



void USART_SendNewLine(void)
{
	USART_SendByte(0x0D);
	USART_SendByte(0x0A);
}



void USART_Initialization(void)
{
	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	// RCC ENABLE ---------------------------------------------------
    RCC_AHBPeriphClockCmd(USARTx_GPIO_CLK, ENABLE);

        // USART2
	//RCC_APB1PeriphClockCmd(USARTx_CLK, ENABLE);
    RCC_APB2PeriphClockCmd(USARTx_CLK, ENABLE);
	
	// GPIO CONFIG --------------------------------------------------
        
        // USART2
	//GPIO_PinAFConfig(USARTx_GPIO, USARTx_PinSource_TX, GPIO_AF_1);
	//GPIO_PinAFConfig(USARTx_GPIO, USARTx_PinSource_RX, GPIO_AF_1); 
        
    GPIO_PinAFConfig(USARTx_GPIO, USARTx_PinSource_TX, GPIO_AF_1);
	GPIO_PinAFConfig(USARTx_GPIO, USARTx_PinSource_RX, GPIO_AF_1);  

	//
	GPIO_InitStructure.GPIO_Pin = USARTx_TX | USARTx_RX;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Init(USARTx_GPIO, &GPIO_InitStructure); 
	
	// USART CONFIG --------------------------------------------------
	USART_InitStructure.USART_BaudRate = USARTx_BaudRate;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_Init(USARTx, &USART_InitStructure);
	//
	USART_ITConfig(USARTx, USART_IT_RXNE, ENABLE);
	//
	USART_Cmd(USARTx, ENABLE);
	
	// NVIC CONFIG ---------------------------------------------------
	NVIC_InitStructure.NVIC_IRQChannel = USARTx_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0; // ???
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	return;
}