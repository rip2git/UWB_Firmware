/*! ----------------------------------------------------------------------------
 * @file	deca_spi.c
 * @brief	SPI access functions
 *
 * @attention
 *
 * Copyright 2013 (c) DecaWave Ltd, Dublin, Ireland.
 *
 * All rights reserved.
 *
 * @author DecaWave
 */
#include <string.h>

#include "deca_spi.h"
#include "deca_device_api.h"
#include "port.h"

volatile uint8 dummyVola;

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: openspi()
 *
 * Low level abstract function to open and initialise access to the SPI device.
 * returns 0 for success, or -1 for error
 */
int openspi(/*SPI_TypeDef* SPIx*/)
{
	// done by port.c, default SPI used is SPI1

	return 0;

} // end openspi()

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: closespi()
 *
 * Low level abstract function to close the the SPI device.
 * returns 0 for success, or -1 for error
 */
int closespi(void)
{
	while (port_SPIx_busy_sending()); //wait for tx buffer to empty

	port_SPIx_disable();

	return 0;

} // end closespi()

/*! ------------------------------------------------------------------------------------------------------------------
 * Function: writetospi()
 *
 * Low level abstract function to write to the SPI
 * Takes two separate byte buffers for write header and write data
 * returns 0 for success, or -1 for error
 */
int writetospi(uint16 headerLength, const uint8 *headerBuffer, uint32 bodylength, const uint8 *bodyBuffer)
{
    int i;

    decamutexon();	
	SPIx_CS_GPIO->BRR = SPIx_CS;    

    for(i = 0; i < headerLength; ++i) {
		*(uint8_t *)&(SPIx->DR) = headerBuffer[i];                            

        // Dummy read from rxfifo
		while ( !(SPIx->SR & SPI_I2S_FLAG_RXNE) )
        	;
		dummyVola = *(__IO uint8_t *)(&(SPIx->DR));      
    }

    for (i = 0; i < bodylength; ++i) {
		*(uint8_t *)&(SPIx->DR) = bodyBuffer[i];
    	
        // Dummy read from rxfifo
		while ( !(SPIx->SR & SPI_I2S_FLAG_RXNE) )	
        	; 
		dummyVola = *(__IO uint8_t *)(&(SPIx->DR));  
    }

	SPIx_CS_GPIO->BSRR = SPIx_CS;	
    decamutexoff();

    return 0;
}


/*! ------------------------------------------------------------------------------------------------------------------
 * Function: readfromspi()
 *
 * Low level abstract function to read from the SPI
 * Takes two separate byte buffers for write header and read data
 * returns the offset into read buffer where first byte of read data may be found,
 * or returns -1 if there was an error
 */
int readfromspi(uint16 headerLength, const uint8 *headerBuffer, uint32 readlength, uint8 *readBuffer)
{
    int i;

    decamutexon();	
	SPIx_CS_GPIO->BRR = SPIx_CS;	

    for (i = 0; i < headerLength; ++i) {      
		*(__IO uint8_t *)(&(SPIx->DR)) = (uint8_t)headerBuffer[i]; 				
		
        // Dummy read from rxfifo
		while ( !(SPIx->SR & SPI_I2S_FLAG_RXNE) )
        	;
		dummyVola = *(__IO uint8_t *)(&(SPIx->DR));
    }
	
    for (i = 0; i < readlength; ++i) {
    	// Dummy write as we read the message body   
		*(__IO uint8_t *)(&(SPIx->DR)) = (uint8_t)0;	
		
		while ( !(SPIx->SR & SPI_I2S_FLAG_RXNE) )	
        	; 	
		readBuffer[i] = *(__IO uint8_t *)(&(SPIx->DR));	
    }
	
	SPIx_CS_GPIO->BSRR = SPIx_CS;	
    decamutexoff();

    return 0;
}
