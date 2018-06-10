#ifndef _DEBUGGER
#define _DEBUGGER

#define USE_DEBUGGER

#ifdef USE_DEBUGGER
#include "stdint.h"

/*! ------------------------------------------------------------------------------------------------------------------
 * @fn: Debugger_SetMode
 *
 * @brief: Provides enable or disable streaming debugging information thru USART
 *
 * NOTE:
 *
 * input parameters
 * @param mode - 0 -disable, 1 -enable streaming
 *
 * no return value
*/
extern void Debugger_SetMode(uint8_t mode);


extern void Debugger_ConstrAndSendBuf(int length, ...);
extern void Debugger_PushSymBack(uint8_t sym);
extern void Debugger_PushArrayBack(const uint8_t *buffer, uint8_t length);
extern void Debugger_SendPreparedBuf();
extern void Debugger_SendNewBuf(const uint8_t *buffer, uint8_t length);
extern void Debugger_SendStr(const char *string);

#else

#define Debugger_SetMode(x) ((void)0)
#define Debugger_ConstrAndSendBuf(x, ...) ((void)0)
#define Debugger_PushSymBack(x) ((void)0)
#define Debugger_PushArrayBack(x, y) ((void)0)
#define Debugger_SendPreparedBuf(x) ((void)0)
#define Debugger_SendNewBuf(x, y) ((void)0)
#define Debugger_SendStr(x) ((void)0)

#endif



#endif
