#ifndef CONFIGFW_H
#define CONFIGFW_H


#include "UserPack.h"


/*! ------------------------------------------------------------------------------------------------------------------
 * @def: ConfigFW_SIZE
 *
 * @brief: size of ConfigFW
 *
*/
#define ConfigFW_SIZE	12


/*! ------------------------------------------------------------------------------------------------------------------
 * @typedef: ConfigFW
 *
 * @brief: Global config of the firmware
 *
*/
static struct {
	struct SW1000_Str {
		uint16_t 	DeviceID;
		uint16_t 	PAN_ID;
		uint8_t		nDevices;
		uint16_t	PollingPeriod;
	} SW1000;
	struct Token_Str {
		uint8_t		TimeSlotDurationMs;
	} Token;
	struct Ranging_Str {
		uint16_t	RespondingDelay;
		uint16_t	FinalDelay;
	} Ranging;
} ConfigFW;


/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: ConfigFW_FromUserPack
 *
 * @brief: Converts structed pack to config.
 *
 * NOTE: 
 *
 * input parameters
 * @param pack - structured data with payload
 *
 * no return value
*/
static inline void ConfigFW_FromUserPack(const UserPack *pack)
{	
	uint8_t i = 0;
	
	ConfigFW.SW1000.DeviceID = pack->Data[i++];
	ConfigFW.SW1000.DeviceID |= pack->Data[i++] << 8;
	ConfigFW.SW1000.PAN_ID = pack->Data[i++];
	ConfigFW.SW1000.PAN_ID |= pack->Data[i++] << 8;
	ConfigFW.SW1000.nDevices = pack->Data[i++];
	ConfigFW.SW1000.PollingPeriod = pack->Data[i++];
	ConfigFW.SW1000.PollingPeriod |= pack->Data[i++] << 8;
	//
	ConfigFW.Token.TimeSlotDurationMs = pack->Data[i++];
	//
	ConfigFW.Ranging.RespondingDelay = pack->Data[i++];
	ConfigFW.Ranging.RespondingDelay |= pack->Data[i++] << 8;
	ConfigFW.Ranging.FinalDelay	= pack->Data[i++];
	ConfigFW.Ranging.FinalDelay	|= pack->Data[i++] << 8;
}


#endif
