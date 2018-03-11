#ifndef mBASETIMER_H
#define mBASETIMER_H


#include <stdint.h>


typedef uint8_t BaseTimer_STATE;


#define BaseTimer_RESET		(0)
#define BaseTimer_SET		(!BaseTimer_RESET)


extern void BaseTimer_Initialization(void);
extern inline void BaseTimer_SetPrescaler(uint16_t prescaler);
extern inline void BaseTimer_SetPeriod(uint16_t period);
extern inline void BaseTimer_Enable(void);
extern inline void BaseTimer_Disable(void);
extern inline void BaseTimer_Reset(void);
extern inline void BaseTimer_Set(uint16_t cnt);
extern inline uint16_t BaseTimer_Get(void);
extern inline BaseTimer_STATE BaseTimer_GetState(void);



#endif