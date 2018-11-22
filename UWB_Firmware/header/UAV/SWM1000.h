#ifndef SWM1000_H_
#define SWM1000_H_



// def to console
//#define SWM1000_ASCII_SYM
// def for enabling filtering
//#define SWM1000_FILTERING

#define SWM1000_PAN_SIZE			65535

#define SWM1000_TIMESLOT_DURATION	1
#define SWM1000_TIMESLOT_MAX_DUR	100
#define SWM1000_PANID				0x1111



extern void SWM1000_Initialization(void);
extern void SWM1000_Loop(void);


#endif
