#ifndef SWM1000_H_
#define SWM1000_H_



#define SWM1000_nDEVICES			10
#define SWM1000_PollingPeriod		1000

// TMP
/*
	Position content:
		0 byte 		- ID
		1/2 byte 	- 1/10 sec
		3/4 byte	- GPS longt
		5/6 byte	- GPS latit
		7/8 byte	- Distance (after 'ds-twr')	
*/

extern void SWM1000_Initialization(void);
extern void SWM1000_Loop(void);


#endif
