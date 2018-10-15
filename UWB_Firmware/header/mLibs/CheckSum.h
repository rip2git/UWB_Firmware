#ifndef CHECKSUM_H
#define CHECKSUM_H

#include <stdint.h>


/*! ------------------------------------------------------------------------------------------------------------------
 * @def: CRCx
 *
 * @brief: Defines that check sum whould be used 
 *
*/
#define CRC8
//#define CRC16

/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: uint16_str
 *
 * @brief: Used to convert a word to an array of bytes
 *
*/
typedef union {
	uint16_t d;
	uint8_t b[2];
} uint16_str;


/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: CheckSum_Initialization
 *
 * @brief: Creates a table(s) of 'CRC codes'
 *
 * NOTE: 
 *
 * input parameters
 *
 * output parameters
 *
 * no return value
*/
extern void CheckSum_Initialization(void);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: CheckSum_GetCRC8
 *
 * @brief: Calculates a crc8 code for an array of bytes
 *
 * NOTE: 
 *
 * input parameters
 * @param: buffer - array of bytes for which a crc8 should be calculated
 * @param: len - size of the buffer 
 *
 * output parameters
 *
 * return value is the crc8 code of the buffer
*/
extern uint8_t CheckSum_GetCRC8(const uint8_t *buffer, uint16_t len);

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: CheckSum_GetCRC16
 *
 * @brief: Calculates a crc16 code for array of bytes
 *
 * NOTE: 
 *
 * input parameters
 * @param: buffer - array of bytes for which a crc16 should be calculated
 * @param: len - size of the buffer 
 *
 * output parameters
 *
 * return value is the crc16 code of the buffer
*/
extern uint16_t CheckSum_GetCRC16(const uint8_t *buffer, uint16_t len);


#endif
