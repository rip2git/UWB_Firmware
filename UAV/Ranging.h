#ifndef RANGING_H_
#define RANGING_H_


#include "Transceiver.h"
#include "MACFrame.h"


#define Ranging_PAYLOAD_SIZE	6


typedef int Ranging_RESULT;


#define Ranging_ERROR 		(-1)
#define Ranging_SUCCESS 	(0)


extern void Ranging_Initialization(void);
extern Ranging_RESULT Ranging_GetDistance(MACHeader_Typedef *header, uint16_t *distance16);
extern Ranging_RESULT Ranging_Initiate(MACHeader_Typedef *header, const uint8_t *payload);


#endif