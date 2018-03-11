#include "CheckSum.h"


//#define CRC8
#define CRC16


#ifdef CRC8
	static uint8_t crc8table[256];
	
	
	
	static inline void _CheckSum_CRC8_MakeTable()
	{
		uint16_t i;
		uint8_t r;
		uint8_t j;

		for (i = 0; i < 256; i++) {
			r = (uint8_t)(i);
			for (j = 0; j < 8; j++) {
				if ((r & 0x80))
					r = (r << 1) ^ 0x31;
				else
					r = r << 1;
			}
			crc8table[i] = r;
		}
	}
	
	
	
	uint8_t CheckSum_GetCRC8(const uint8_t *buffer, uint16_t len)
	{
		uint8_t crc;
		crc = 0xFF;
		while (len--) {
			crc = crc8table[ (crc ^ *buffer++) & 0xFF ];
		}
		crc ^= 0xFF;
		return crc;
	}
#endif
#ifdef CRC16
	static uint16_t crc16table[256];
	
	
	
	static inline void _CheckSum_CRC16_MakeTable()
	{
		uint16_t i;
		uint16_t r;
		uint8_t j;

		for (i = 0; i < 256; i++) {
			r = i << 8;
			for (j = 0; j < 8; j++) {
				if (r & (1 << 15))
					r = (r << 1) ^ 0x8005;
				else
					r = r << 1;
			}
			crc16table[i] = r;
		}
	}
	
	
	
	uint16_t CheckSum_GetCRC16(const uint8_t *buffer, uint16_t len)
	{
		uint16_t crc;
		crc = 0xFFFF;
		while (len--) {
			crc = crc16table[ ((crc >> 8) ^ *buffer++) & 0xFF ] ^ (crc << 8);
		}
		crc ^= 0xFFFF;
		return crc;
	}
#endif







void CheckSum_Initialization(void)
{
#ifdef CRC8	
	_CheckSum_CRC8_MakeTable();
#endif
#ifdef CRC16	
	_CheckSum_CRC16_MakeTable();	
#endif
}

