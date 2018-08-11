#ifndef PORT_H_
#define PORT_H_


#include "stm32f0xx.h"	
	
#include "SystemTimer.h"
#include "EventTimer.h"
  
	  
/* Define our wanted value of CLOCKS_PER_SEC so that we have a 10 millisecond
 * tick timer. */
#undef CLOCKS_PER_SEC
#define CLOCKS_PER_SEC 1000
	
/*! ------------------------------------------------------------------------------------------------------------------
 * @def: 
 *
 * @brief: 
 *
*/  
#define DW1000_RSTn					GPIO_Pin_11
#define DW1000_RSTn_GPIO			GPIOA   
  
/*! ------------------------------------------------------------------------------------------------------------------
 * @def: 
 *
 * @brief: 
 *
*/  
#define SPIx_PRESCALER				SPI_BaudRatePrescaler_2 //SPI_BaudRatePrescaler_8
#define SPIx				        SPI1
#define SPIx_GPIO					GPIOA
#define SPIx_CS						GPIO_Pin_4
#define SPIx_CS_GPIO				GPIOA
#define SPIx_SCK					GPIO_Pin_5
#define SPIx_MISO					GPIO_Pin_6
#define SPIx_MOSI					GPIO_Pin_7
#define SPIx_SCK_SOURCE				GPIO_PinSource5
#define SPIx_MOSI_SOURCE			GPIO_PinSource7
#define SPIx_MISO_SOURCE			GPIO_PinSource6
  
/*! ------------------------------------------------------------------------------------------------------------------
 * @def: 
 *
 * @brief: 
 *
*/  
#define DECAIRQ                     GPIO_Pin_0
#define DECAIRQ_GPIO                GPIOB
#define DECAIRQ_CLK                	RCC_AHBPeriph_GPIOB
#define DECAIRQ_EXTI                EXTI_Line0
#define DECAIRQ_EXTI_PIN            EXTI_PinSource0
#define DECAIRQ_EXTI_PORT           EXTI_PortSourceGPIOB
#define DECAIRQ_EXTI_IRQn           EXTI0_1_IRQn
#define DECAIRQ_EXTI_USEIRQ         ENABLE
#define DECAIRQ_EXTI_NOIRQ          DISABLE

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: 
 *
 * @brief: 
 *
*/  
#define port_SPIx_busy_sending()		(SPI_I2S_GetFlagStatus((SPIx),(SPI_I2S_FLAG_TXE))==(RESET))
#define port_SPIx_no_data()				(SPI_I2S_GetFlagStatus((SPIx),(SPI_I2S_FLAG_RXNE))==(RESET))
#define port_SPIx_send_data(x)			SPI_I2S_SendData((SPIx),(x))
#define port_SPIx_receive_data()		SPI_I2S_ReceiveData(SPIx)
#define port_SPIx_disable()				SPI_Cmd(SPIx,DISABLE)
#define port_SPIx_enable()              SPI_Cmd(SPIx,ENABLE)
#define port_SPIx_set_chip_select()		GPIO_SetBits(SPIx_CS_GPIO,SPIx_CS)
#define port_SPIx_clear_chip_select()	GPIO_ResetBits(SPIx_CS_GPIO,SPIx_CS)

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: 
 *
 * @brief: 
 *
*/  
ITStatus EXTI_GetITEnStatus(uint32_t x);

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: 
 *
 * @brief: 
 *
*/  
#define port_GetEXT_IRQStatus()             EXTI_GetITEnStatus(DECAIRQ_EXTI_IRQn)
#define port_DisableEXT_IRQ()               NVIC_DisableIRQ(DECAIRQ_EXTI_IRQn)
#define port_EnableEXT_IRQ()                NVIC_EnableIRQ(DECAIRQ_EXTI_IRQn)
#define port_CheckEXT_IRQ()                 GPIO_ReadInputDataBit(DECAIRQ_GPIO, DECAIRQ)

/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: 
 *
 * @brief: 
 *
*/  
typedef void (*port_deca_isr_t)(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @def: 
 *
 * @brief: 
 *
*/  
extern port_deca_isr_t port_deca_isr;

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: 
 *
 * @brief: 
 *
*/ 
int NVIC_DisableDECAIRQ(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn peripherals_init()
 *
 * @brief Initialise all peripherals.
 *
 * @param none
 *
 * @return none
 */
void peripherals_init (void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn port_set_deca_isr()
 *
 * @brief This function is used to install the handling function for DW1000 IRQ.
 *
 * NOTE:
 *   - As EXTI9_5_IRQHandler does not check that port_deca_isr is not null, the user application must ensure that a
 *     proper handler is set by calling this function before any DW1000 IRQ occurs!
 *   - This function makes sure the DW1000 IRQ line is deactivated while the handler is installed.
 *
 * @param deca_isr function pointer to DW1000 interrupt handler to install
 *
 * @return none
 */
void port_set_deca_isr(port_deca_isr_t deca_isr);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn spi_set_rate_low()
 *
 * @brief Set SPI rate to less than 3 MHz to properly perform DW1000 initialisation.
 *
 * @param none
 *
 * @return none
 */
void spi_set_rate_low (void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn spi_set_rate_high()
 *
 * @brief Set SPI rate as close to 20 MHz as possible for optimum performances.
 *
 * @param none
 *
 * @return none
 */
void spi_set_rate_high (void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn reset_DW1000()
 *
 * @brief 
 *
 * @param none
 *
 * @return none
 */
void reset_DW1000(void);


#endif /* PORT_H_ */
