
#include "port.h"
#include "deca_sleep.h"



#define rcc_init(x)				RCC_Configuration(x)
#define systick_init(x)			SysTick_Configuration(x)
#define rtc_init(x)				RTC_Configuration(x)
#define interrupt_init(x)		IRQ_Configuration(x)
#define spi_init(x)				SPI_Configuration(x)
#define timer_init(x)			BaseTimer_Initialization(x)


/* DW1000 IRQ handler definition. */
port_deca_isr_t port_deca_isr = 0;



int No_Configuration(void)
{
	return -1;
}



int SysTick_Configuration(void)
{
	if (SysTick_Config(SystemCoreClock / CLOCKS_PER_SEC))
	{
		/* Capture error */
		while (1);
	}
	NVIC_SetPriority (SysTick_IRQn, 3);

	return 0;
}



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
void port_set_deca_isr(port_deca_isr_t deca_isr)
{
    /* Check DW1000 IRQ activation status. */
    ITStatus en = port_GetEXT_IRQStatus();

    /* If needed, deactivate DW1000 IRQ during the installation of the new handler. */
    if (en)
    {
        port_DisableEXT_IRQ();
    }
    port_deca_isr = deca_isr;
    if (en)
    {
        port_EnableEXT_IRQ();
    }
}



int NVIC_DisableDECAIRQ(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;

	/* Configure EXTI line */
	EXTI_InitStructure.EXTI_Line = DECAIRQ_EXTI;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;	//MPW3 IRQ polarity is high by default
	EXTI_InitStructure.EXTI_LineCmd = DECAIRQ_EXTI_NOIRQ;
	EXTI_Init(&EXTI_InitStructure);

	return 0;
}



int IRQ_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_AHBPeriphClockCmd(DECAIRQ_CLK, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);
	
	// Enable GPIO used as DECA IRQ for interrupt
	GPIO_InitStructure.GPIO_Pin = DECAIRQ;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;	
    // IRQ pin should be Pull Down to prevent unnecessary EXT IRQ while DW1000 goes to sleep mode
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(DECAIRQ_GPIO, &GPIO_InitStructure);

	// Connect EXTI Line to GPIO Pin 
    SYSCFG_EXTILineConfig(DECAIRQ_EXTI_PORT, DECAIRQ_EXTI_PIN);

	// Configure EXTI line 
	EXTI_InitStructure.EXTI_Line = DECAIRQ_EXTI;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
	EXTI_InitStructure.EXTI_LineCmd = DECAIRQ_EXTI_USEIRQ;
	EXTI_Init(&EXTI_InitStructure);

	// Enable and set EXTI Interrupt to the lowest priority
	NVIC_InitStructure.NVIC_IRQChannel = DECAIRQ_EXTI_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DECAIRQ_EXTI_USEIRQ;
	NVIC_Init(&NVIC_InitStructure);

	return 0;
}



/**
  * @brief  Checks whether the specified EXTI line is enabled or not.
  * @param  EXTI_Line: specifies the EXTI line to check.
  *   This parameter can be:
  *     @arg EXTI_Linex: External interrupt line x where x(0..19)
  * @retval The "enable" state of EXTI_Line (SET or RESET).
  */
ITStatus EXTI_GetITEnStatus(uint32_t EXTI_Line)
{
        ITStatus bitstatus = RESET;
        uint32_t enablestatus = 0;
        /* Check the parameters */
        assert_param(IS_GET_EXTI_LINE(EXTI_Line));

        enablestatus =  EXTI->IMR & EXTI_Line;
        if (enablestatus != (uint32_t)RESET)
        {
          bitstatus = SET;
        }
        else
        {
          bitstatus = RESET;
        }
        return bitstatus;
}



int RCC_Configuration(void)
{
	ErrorStatus HSEStartUpStatus;
	RCC_ClocksTypeDef RCC_ClockFreq;
	
	RCC_DeInit();    
	RCC_HSEConfig(RCC_HSE_ON);
	HSEStartUpStatus = RCC_WaitForHSEStartUp();
	
	RCC_GetClocksFreq(&RCC_ClockFreq);
        
	if (HSEStartUpStatus != ERROR) {
		RCC_PREDIV1Config(RCC_PREDIV1_Div3);
		RCC_PLLConfig(RCC_PLLSource_PREDIV1, RCC_PLLMul_10); // 12 / 3 * 10 = 40 MHz
	} else {
		RCC_PLLConfig(RCC_PLLSource_HSI, RCC_PLLMul_10); // 4 * 10 = 40 MHz		
	}
	
	RCC_PLLCmd(ENABLE);
	RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);
	
	while (RCC_GetSYSCLKSource() != 0x08) // PLL
		;
	
	RCC_HCLKConfig(RCC_SYSCLK_Div1); // ahb - core, memory, /8 = sys_timer 
	RCC_PCLKConfig(RCC_HCLK_Div1); // apb - usart, spi
	
	RCC_GetClocksFreq(&RCC_ClockFreq);		
	
	RCC_USARTCLKConfig(RCC_USART1CLK_HSI);	
	
	// Enable GPIOs clocks
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB, ENABLE);

	return 0;
}




void SPI_ChangeRate(uint16_t scalingfactor)
{
	uint16_t tmpreg = 0;

	/* Get the SPIx CR1 value */
	tmpreg = SPIx->CR1;

	/*clear the scaling bits*/
	tmpreg &= 0xFFC7;

	/*set the scaling bits*/
	tmpreg |= scalingfactor;

	/* Write to SPIx CR1 */
	SPIx->CR1 = tmpreg;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn spi_set_rate_low()
 *
 * @brief Set SPI rate to less than 3 MHz to properly perform DW1000 initialisation.
 *
 * @param none
 *
 * @return none
 */
void spi_set_rate_low (void)
{
    SPI_ChangeRate(SPI_BaudRatePrescaler_16);
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn spi_set_rate_high()
 *
 * @brief Set SPI rate as close to 20 MHz as possible for optimum performances.
 *
 * @param none
 *
 * @return none
 */
void spi_set_rate_high (void)
{
    SPI_ChangeRate(SPI_BaudRatePrescaler_2);
}



int SPI_Configuration(void)
{
	SPI_InitTypeDef SPI_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1, ENABLE);

	SPI_I2S_DeInit(SPIx);

	// SPIx Mode setup
	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_Low;	
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_1Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPIx_PRESCALER;
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;

	SPI_Init(SPIx, &SPI_InitStructure);

	// SPIx SCK and MOSI pin setup
    GPIO_PinAFConfig(SPIx_GPIO, SPIx_SCK_SOURCE, GPIO_AF_0);
	GPIO_PinAFConfig(SPIx_GPIO, SPIx_MOSI_SOURCE, GPIO_AF_0);
	GPIO_PinAFConfig(SPIx_GPIO, SPIx_MISO_SOURCE, GPIO_AF_0);        
        
	GPIO_InitStructure.GPIO_Pin = SPIx_SCK | SPIx_MOSI | SPIx_MISO;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(SPIx_GPIO, &GPIO_InitStructure);
        
        
	// SPIx CS pin setup
	GPIO_InitStructure.GPIO_Pin = SPIx_CS;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;

	GPIO_Init(SPIx_CS_GPIO, &GPIO_InitStructure);

        
	// Disable SPIx SS Output
	SPI_SSOutputCmd(SPIx, DISABLE);

	// Enable SPIx
	SPI_Cmd(SPIx, ENABLE);

	// Set CS high
	GPIO_SetBits(SPIx_CS_GPIO, SPIx_CS);

    return 0;
}



void reset_DW1000(void)
{
    volatile uint16_t ts = 8000;
	GPIO_InitTypeDef GPIO_InitStructure;

	// Enable GPIO used for DW1000 reset
	GPIO_InitStructure.GPIO_Pin = DW1000_RSTn;
   	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(DW1000_RSTn_GPIO, &GPIO_InitStructure);

	//drive the RSTn pin low
	GPIO_ResetBits(DW1000_RSTn_GPIO, DW1000_RSTn);
	while (ts--)
		;

	//put the pin back to tri-state ... as input
	GPIO_InitStructure.GPIO_Pin = DW1000_RSTn;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_Init(DW1000_RSTn_GPIO, &GPIO_InitStructure);
	
    sleep_10ms(1); 
}



int is_IRQ_enabled(void)
{
	return ((   NVIC->ISER[((uint32_t)(DECAIRQ_EXTI_IRQn) >> 5)]
	           & (uint32_t)0x01 << (DECAIRQ_EXTI_IRQn & (uint8_t)0x1F)  ) ? 1 : 0) ;
}



unsigned long portGetTickCnt(void)
{
	return time32_incr;
}



/*! ------------------------------------------------------------------------------------------------------------------
 * @fn peripherals_init()
 *
 * @brief Initialise all peripherals.
 *
 * @param none
 *
 * @return none
 */
void peripherals_init (void)
{
	rcc_init();
	interrupt_init();
	systick_init();
    spi_init();	
	timer_init();
}
