#ifndef TOKEN_
#define TOKEN_

#include "MACFrame.h"
#include <stdint.h>


typedef int Token_RESULT;
typedef uint8_t Token_BOOL;


#define Token_FAIL			(-1)
#define Token_SUCCESS		0



#define Token_FALSE		(0)
#define Token_TRUE		(!Token_FALSE)



extern Token_RESULT Token_Transfer(MACHeader_Typedef *header);
extern Token_RESULT Token_Receipt(MACHeader_Typedef *header, const uint8_t *buffer);
extern Token_RESULT Token_Generate(MACHeader_Typedef *header);
extern Token_RESULT Token_ImmediateReceipt(MACHeader_Typedef *header);
extern Token_BOOL Token_isCaptured();
extern void Token_SetMaxID(uint8_t ID);

#endif