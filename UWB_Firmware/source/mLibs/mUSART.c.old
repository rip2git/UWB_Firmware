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



#define USARTx_RX_BUFFER_SIZE	255
#define USARTx_TX_BUFFER_SIZE	255

static const uint32_t USARTx_ERROR_FLAGS = USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_ORE | USART_FLAG_PE;
volatile uint8_t dummyTmp;

volatile uint8_t USARTx_ReceivingError;
//
volatile uint8_t USARTx_RxBuffer[USARTx_RX_BUFFER_SIZE];
volatile uint16_t USARTx_RxWr_Idx = 0, USARTx_RxRd_Idx = 0;
volatile uint16_t USARTx_RxCounter = 0;
//
volatile uint8_t USARTx_TxBuffer[USARTx_TX_BUFFER_SIZE];
volatile uint16_t USARTx_TxWr_Idx = 0, USARTx_TxRd_Idx = 0;
volatile uint16_t USARTx_TxCounter = 0;



inline uint16_t USART_GetRxCounter(void)
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



inline void USART_ResetRxBuffer(void)
{
	USARTx_RxWr_Idx = 0;		
	USARTx_RxCounter = 0;
}



uint8_t USART_GetByte(void)
{
	uint8_t data;
	
	while (USART_GetRxCounter() == USART_RX_EMPTY && USARTx_ReceivingError == 0)
		;
	
	if (USARTx_ReceivingError) 
		return 0;
	
	data = USARTx_RxBuffer[USARTx_RxRd_Idx++];
	
	if (USARTx_RxRd_Idx == USARTx_RX_BUFFER_SIZE) // buffer cycle
		USARTx_RxRd_Idx = 0;
		
	USARTx->CR1 &= ~USART_CR1_RXNEIE; // rxne interrupt disable	
	--USARTx_RxCounter;
	USARTx->CR1 |= USART_CR1_RXNEIE; // rxne interrupt enable
	
	return data;
}



void USART_SendByte(const uint8_t c)
{
	while (USARTx_TxCounter == USARTx_TX_BUFFER_SIZE) // buffer is full
		; 	
	USART1->CR1 &= ~USART_CR1_TXEIE; // txe interrupt disable
	
	// buffer not empty or transmitting
	if (USARTx_TxCounter || !(USART1->ISR & USART_FLAG_TXE)) {
		USARTx_TxBuffer[USARTx_TxWr_Idx++] = c;
		++USARTx_TxCounter;
		if (USARTx_TxWr_Idx == USARTx_TX_BUFFER_SIZE) // buffer cycle
			USARTx_TxWr_Idx = 0;			
		USARTx->CR1 |= USART_CR1_TXEIE; // txe interrupt enable
		
	} else {
		USARTx->TDR = c;
	}
}



void USART_SendBuffer(const uint8_t *buffer, uint16_t length)
{
	uint16_t i;
	for (i = 0; i < length; ++i)
		USART_SendByte(buffer[i]);	
}



void USART_SendString(const char *string)
{	
#ifdef MAIN_DEBUG	
/* TODO: testing space. Include testing code here */
	uint16_t i = 0;
	while (string[i] != '\0') {
		
		while ( !(USARTx->ISR & USART_FLAG_TXE) )
			;
		
		USARTx->TDR = string[i];
		
		i++;
	}
/* TODO: end of testing code*/	
#else
	uint16_t i = 0;
	while (string[i] != '\0') {
		USART_SendByte(string[i]);
		i++;
	}
#endif	
}



void USARTx_IRQHandler(void)
{ 
	// ERROR
	if ( USARTx->ISR & USARTx_ERROR_FLAGS ) {
		dummyTmp = (uint8_t)(USARTx->RDR);		
		USARTx->ICR |= (USARTx->ISR & USARTx_ERROR_FLAGS);
		USARTx_ReceivingError++;
	}	

	// RECEIVE 
	if ( USARTx->ISR & USART_FLAG_RXNE ) {                   
		USARTx_RxBuffer[USARTx_RxWr_Idx++] = (uint8_t)(USARTx->RDR);
		
		if (USARTx_RxWr_Idx == USARTx_RX_BUFFER_SIZE)
			USARTx_RxWr_Idx = 0;
		if (++USARTx_RxCounter == USARTx_RX_BUFFER_SIZE) 		
			USARTx_RxCounter = 0;			
		
	}

#ifdef MAIN_DEBUG
/* TODO: testing space. Include testing code here */	
/* TODO: end of testing code*/		
#else		
	// TRANSMIT 
	if ( USARTx->ISR & USART_FLAG_TXE ) {
		if (USARTx_TxCounter) {
			USARTx_TxCounter--;			
			USARTx->TDR = USARTx_TxBuffer[USARTx_TxRd_Idx++];
			if (USARTx_TxRd_Idx == USARTx_TX_BUFFER_SIZE)
				USARTx_TxRd_Idx = 0;				
		} else {
			USARTx->CR1 &= ~USART_CR1_TXEIE; // txe interrupt disable         
		}
	}
#endif	
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
	//
	USART_ITConfig(USARTx, USART_IT_RXNE | USART_IT_ERR, ENABLE); // txe enable/disable while working
	//
	USART_Cmd(USARTx, ENABLE);
	
	// NVIC CONFIG ---------------------------------------------------
	NVIC_InitStructure.NVIC_IRQChannel = USARTx_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPriority = 0; // ???
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	return;
}