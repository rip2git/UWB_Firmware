#ifndef SWM1000_H_
#define SWM1000_H_



#define SWM1000_nDEVICES			10

#define SWM1000_PollingPeriod		100 // in 10 of ms 
#define SWM1000_TimeSlotDuration	40


/*
	Position content:
		0 byte 		- ID
		1/2 byte 	- 1/10 sec
		3/4 byte	- GPS longt
		5/6 byte	- GPS latit
		7/8 byte	- Distance (after 'ds-twr')	
*/
/*#define SWM1000_RNG_ID_OFFSET			0
#define SWM1000_RNG_TENTH_SEC_OFFSET	1
#define SWM1000_RNG_GPS_LONGT_OFFSET	3
#define SWM1000_RNG_GPS_LATIT_OFFSET	5
#define SWM1000_RNG_DISTANCE_OFFSET		7

#define SWM1000_RNG_PAYLOAD_OFFSET		1

#define SWM1000_RNG_ID_SIZE				1
#define SWM1000_RNG_DISTANCE_SIZE		2
#define SWM1000_RNG_PAYLOAD_SIZE 		9*/


extern void SWM1000_Initialization(void);
extern void SWM1000_Loop(void);


#endif
