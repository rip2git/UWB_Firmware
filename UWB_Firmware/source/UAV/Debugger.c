
#include "Debugger.h"


#ifdef USE_DEBUGGER
#include <stdio.h>
#include <stdarg.h>
#include "MACFrame.h"
#include "USARTHandler.h"
#include "string.h"
#include "deca_sleep.h"



static uint8_t _Debugger_mode = 0;
static uint8_t _Debugger_it = 0;
static UserPack _Debugger_pack;



void Debugger_ConstrAndSendBuf(int length, ...)
{
	if (_Debugger_mode != 0) {
		va_list arguments;
		va_start(arguments, length);

		for (int i = 0; i < length; ++i)
			_Debugger_pack.Data[i] = (uint8_t)(va_arg(arguments, int));

		if (length < UserPack_DATA_MAX_SIZE) {
			_Debugger_pack.FCmd.cmd = UserPack_Cmd_Service;
			_Debugger_pack.SCmd.cmd = UserPack_Cmd_Service;
			_Debugger_pack.TotalSize = length;
			memcpy((void*)_Debugger_pack.Data, _Debugger_pack.Data, length);
			USARTHandler_Send(&_Debugger_pack);
		}
		va_end(arguments);
		_Debugger_it = 0;
	}
}



void Debugger_PushSymBack(uint8_t sym)
{
	if (_Debugger_mode != 0)
		_Debugger_pack.Data[_Debugger_it++] = sym;
}



void Debugger_PushArrayBack(const uint8_t *buffer, uint8_t length)
{
	if (_Debugger_mode != 0)
		for (int i = 0; i < length; ++i)
			_Debugger_pack.Data[_Debugger_it++] = buffer[i];
}



void Debugger_SendPreparedBuf()
{
	if (_Debugger_mode != 0) {
		_Debugger_pack.FCmd.cmd = UserPack_Cmd_Service;
		_Debugger_pack.SCmd.cmd = UserPack_Cmd_Service;
		_Debugger_pack.TotalSize = _Debugger_it;
		memcpy((void*)_Debugger_pack.Data, _Debugger_pack.Data, _Debugger_it);
		USARTHandler_Send(&_Debugger_pack);
		_Debugger_it = 0;
	}
}



void Debugger_SendNewBuf(const uint8_t *buffer, uint8_t length)
{
	if (_Debugger_mode != 0) {
		if (length < UserPack_DATA_MAX_SIZE) {
			_Debugger_pack.FCmd.cmd = UserPack_Cmd_Service;
			_Debugger_pack.SCmd.cmd = UserPack_Cmd_Service;
			_Debugger_pack.TotalSize = length;
			memcpy((void*)_Debugger_pack.Data, buffer, length);
			USARTHandler_Send(&_Debugger_pack);
		}
		_Debugger_it = 0;
	}
}



void Debugger_SendStr(const char *string)
{
	if (_Debugger_mode != 0) {
		uint8_t i = strlen(string);
		if (i < UserPack_DATA_MAX_SIZE) {
			_Debugger_pack.FCmd.cmd = UserPack_Cmd_Service;
			_Debugger_pack.SCmd.cmd = UserPack_Cmd_Service;
			_Debugger_pack.TotalSize = i;
			memcpy((void*)_Debugger_pack.Data, string, i);
			USARTHandler_Send(&_Debugger_pack);
		}
		_Debugger_it = 0;
	}
}



void Debugger_SetMode(uint8_t mode)
{
	_Debugger_mode = mode;
}
#endif



