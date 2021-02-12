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

//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------

#include "Types.h"
#include "string.h"
#include "statemachine.h"
#include "startup.h"
#include "IOExpander.h"
#include "NVM.h"
#include "ticktimer.h"
#include "can.h"
#include "CANTxManager.h"
#include "PRNG.h"
#include "IOControl.h"
#include "InfoTable.h"
#include <stdio.h>
//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// User defined types
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Module variable declarations
//-----------------------------------------------------------------------------

tIOX gIox; 			// extern lives in ioexpander.h

//-----------------------------------------------------------------------------
// Module function declarations
//-----------------------------------------------------------------------------

static void Startup_SetHardwareDependentTypes(void);

//-----------------------------------------------------------------------------
// Module constants
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function   : Startup
// Purpose    : Initialises all the modules of the IO Expander
// Parameters : None
// Return     : None
// Notes      : None
//-----------------------------------------------------------------------------
void Startup(void)
{
	SM_Init();

	gIox.ulSerialNumber = NVM_ReadSerialNumber();

	Startup_SetHardwareDependentTypes();

	//Configure CAN bus
	if (CAN_Init() == 0)
	{
		printf("CAN_CONFIG_FAILED\n");
	}

	IOControl_Init();

	PRNG_Init(gIox.ulSerialNumber);			// seed the PRNG

	CANTxManager_Init();

}

// ****************************************************************
// *********************** STATIC METHODS *************************
// ****************************************************************

static void Startup_SetHardwareDependentTypes(void)
{
	gIox.bIoxType = IOX_TYPE_TEST;
	gIox.bHardwareVersion = IOX_HARDWARE_VERSION_TEST;
}

// End of file

