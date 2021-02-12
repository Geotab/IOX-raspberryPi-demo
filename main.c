//-----------------------------------------------------------------------------
//By using and accessing this material you agree to the terms of the Sample Code
//License Agreement found here. If you do not agree you may not access 
//or otherwise use this information.
//-----------------------------------------------------------------------------
// Module Notes:
// =============
//
// Notes
//
//-----------------------------------------------------------------------------
#include "Types.h"
#include "startup.h"
#include "statemachine.h"

int main(void)
{
	Startup();

	SM_Run();

	return 0;

}

// end of file
