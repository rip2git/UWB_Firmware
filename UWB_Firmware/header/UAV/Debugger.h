#ifndef _DEBUGGER
#define _DEBUGGER



#ifdef USE_DEBUGGER
#include "stdint.h"

extern void Debugger_ConstrAndSendBuf(int length, ...);
extern void Debugger_PushSymBack(uint8_t sym);
extern void Debugger_PushArrayBack(const uint8_t *buffer, uint8_t length);
extern void Debugger_SendPreparedBuf();
extern void Debugger_SendNewBuf(const uint8_t *buffer, uint8_t length);
extern void Debugger_SendStr(const char *string);

#else

#define Debugger_ConstrAndSendBuf(x, ...) ((void)0)
#define Debugger_PushSymBack(x) ((void)0)
#define Debugger_PushArrayBack(x, y) ((void)0)
#define Debugger_SendPreparedBuf(x) ((void)0)
#define Debugger_SendNewBuf(x, y) ((void)0)
#define Debugger_SendStr(x) ((void)0)

#endif



#endif
