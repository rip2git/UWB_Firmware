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
    int i = 0;
    decaIrqStatus_t stat;

    stat = decamutexon();      
    GPIO_ResetBits(SPIx_CS_GPIO, SPIx_CS);
    

    for(i = 0; i < headerLength; ++i) {
        while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET)
          ;     	
        SPI_SendData8(SPIx, headerBuffer[i]); 
                      

        // Dummy read as we write the header 
        while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_BSY) != RESET)
          ;
        SPI_ReceiveData8(SPIx);
    }

    for (i = 0; i < bodylength; ++i) {
        while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET)
          ;
        SPI_SendData8(SPIx, bodyBuffer[i]);
    	
        // Dummy read
        while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_BSY) != RESET)
          ; 
        SPI_ReceiveData8(SPIx);
    }

    GPIO_SetBits(SPIx_CS_GPIO, SPIx_CS);
    decamutexoff(stat) ;

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
    int i = 0;
    decaIrqStatus_t stat;

    stat = decamutexon();    
    GPIO_ResetBits(SPIx_CS_GPIO, SPIx_CS);  

    for (i = 0; i < headerLength; ++i) {      
        while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET)
          ;     	
        SPI_SendData8(SPIx, headerBuffer[i]); 
                      

        // Dummy read as we write the header 
        while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_BSY) != RESET)
          ;
        SPI_ReceiveData8(SPIx);
    }  

    for (i = 0; i < readlength; ++i) {
    	// Dummy write as we read the message body    
        while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_TXE) == RESET)
          ;
        SPI_SendData8(SPIx, 0);

        while (SPI_I2S_GetFlagStatus(SPIx, SPI_I2S_FLAG_BSY) != RESET)
          ; 	
        readBuffer[i] = SPI_ReceiveData8(SPIx);
    }
   
    GPIO_SetBits(SPIx_CS_GPIO, SPIx_CS);
    decamutexoff(stat) ;

    return 0;
}
