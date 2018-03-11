#ifndef CHECKSUM_H
#define CHECKSUM_H


#include <stdint.h>

typedef union {
	uint16_t d;
	uint8_t b[2];
} uint16_str;


extern void CheckSum_Initialization(void);
extern uint8_t CheckSum_GetCRC8(const uint8_t *buffer, uint16_t len);
extern uint16_t CheckSum_GetCRC16(const uint8_t *buffer, uint16_t len);


#endif