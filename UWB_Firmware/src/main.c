
#include "SWM1000.h"

// to debug under gdb need to change ENTRY(_start) to ENTRY(__reset_hardware) in the sections.ld script

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wmissing-declarations"
#pragma GCC diagnostic ignored "-Wreturn-type"
int main(int argc, char* argv[])
{
	SWM1000_Initialization();
	SWM1000_Loop();
	
	return 1;
}
#pragma GCC diagnostic pop
