/**
  ******************************************************************************
  * @file    SPI/SPI_MSD/stm32f0xx_it.c 
  * @author  MCD Application Team
  * @version V1.4.0
  * @date    24-July-2014
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f0xx_it.h"
#include "port.h"
#include "mUSART.h"

/** @addtogroup STM32F0xx_StdPeriph_Examples
  * @{
  */

/** @addtogroup SPI_MSD
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M0 Processor Exceptions Handlers                         */
/******************************************************************************/

/* Tick timer count. */
volatile unsigned long time32_incr;

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
	USART_SendString("HARD FAULT!");
	/* Go to infinite loop when Hard Fault exception occurs */
	while (1)
		;
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	time32_incr++;
}

/******************************************************************************/
/*                 STM32F0xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f0xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn EXTI0_1_IRQHandler()
 *
 * @brief Handler for DW1000 IRQ.
 *
 * @param none
 *
 * @return none
 */
void EXTI0_1_IRQHandler(void)
{
    do
    {
        port_deca_isr();
    } while (port_CheckEXT_IRQ() == 1);	
    
    EXTI_ClearITPendingBit(DECAIRQ_EXTI);
}


extern volatile uint8_t BaseTimer_Event;
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn TIM6_DAC_IRQHandler()
 *
 * @brief Basic timer
 *
 * @param none
 *
 * @return none
 */
void TIM6_DAC_IRQHandler(void)
{
	if ((TIM6->SR & TIM_IT_Update) != (uint16_t)RESET)
	{
		TIM6->SR = (uint16_t)~TIM_IT_Update;
		BaseTimer_Event++;		
	}
}


/*! ------------------------------------------------------------------------------------------------------------------
 * @variables for USART1_IRQHandler()
 *
 * @brief 
 *
 * @param none
 *
 * @return none
 */
#define mUSARTx						USART1
#define mUSARTx_IRQHandler(x)		USART1_IRQHandler(x)

extern const uint16_t USARTx_RX_BUFFER_SIZE;
extern const uint16_t USARTx_TX_BUFFER_SIZE;

extern volatile uint8_t USARTx_ReceivingError;

extern volatile uint8_t USARTx_RxBuffer[];
extern volatile uint16_t USARTx_RxWr_Idx, USARTx_RxRd_Idx;
extern volatile uint16_t USARTx_RxCounter;
//
extern volatile uint8_t USARTx_TxBuffer[];
extern volatile uint16_t USARTx_TxWr_Idx, USARTx_TxRd_Idx;
extern volatile uint16_t USARTx_TxCounter;

static const uint32_t USARTx_ERROR_FLAGS = USART_FLAG_NE | USART_FLAG_FE | USART_FLAG_ORE | USART_FLAG_PE;
volatile uint8_t dummyTmp;
/*! ------------------------------------------------------------------------------------------------------------------
 * @fn mUSARTx_IRQHandler()
 *
 * @brief 
 *
 * @param none
 *
 * @return none
 */
volatile uint16_t tmp_vola1[3];
void mUSARTx_IRQHandler(void)
{ 
	// ERROR
	if ( mUSARTx->ISR & USARTx_ERROR_FLAGS ) {
		dummyTmp = (uint8_t)(mUSARTx->RDR);		
		mUSARTx->ICR = USARTx_ERROR_FLAGS;
		USARTx_ReceivingError++;
	}
	
	// RECEIVE 
	if ( mUSARTx->ISR & USART_FLAG_RXNE ) {                   
		USARTx_RxBuffer[USARTx_RxWr_Idx++] = (uint8_t)(mUSARTx->RDR);
		
		if (USARTx_RxWr_Idx == USARTx_RX_BUFFER_SIZE)
			USARTx_RxWr_Idx = 0;
		if (++USARTx_RxCounter == USARTx_RX_BUFFER_SIZE) 		
			USARTx_RxCounter = 0;			
		
	}
	
	// TRANSMIT 
	if ( mUSARTx->ISR & USART_FLAG_TXE ) {
		if (USARTx_TxCounter) {
			USARTx_TxCounter--;			
			mUSARTx->TDR = USARTx_TxBuffer[USARTx_TxRd_Idx++];
			if (USARTx_TxRd_Idx == USARTx_TX_BUFFER_SIZE)
				USARTx_TxRd_Idx = 0;				
		} else {
			mUSARTx->CR1 &= ~USART_CR1_TXEIE; // txe interrupt disable
  			//USART_ITConfig(mUSARTx, USART_IT_TXE, DISABLE);                  
		}
	}
}



/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
