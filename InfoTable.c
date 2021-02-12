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
#include "CANTxManager.h"

#include "InfoTable.h"
#include <stdio.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// User defined types
//-----------------------------------------------------------------------------

typedef enum
{
	GO_STATUS_IGNITION = 0,

	GO_STATUS_END_OF_LIST
} GoStatusTypes_t;

//-----------------------------------------------------------------------------
// Module variable declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Module function declarations
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Module constants
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function definitions
//-----------------------------------------------------------------------------

// ****************************************************************
// *********************** STATIC METHODS *************************
// ****************************************************************

// ****************************************************************
// ************** START of InfoTable Message Handlers *************
// ****************************************************************

void InfoTable_StoreStatus(IOExpanderMsg_t * IOExpanderMsg)
{
	int i;

	//Print a received message to user console
	printf("Received message_id = 0x%x: Status Information from GO - ", MSG_ID_GO_STATUS_INFORMATION);

	for(i = 0; i < IOExpanderMsg->bLength; i++)
	{
		printf("0x%x ", IOExpanderMsg->abData[i]);
	}
	printf("\n");

	uint16_t usType;	// Presumed present when read, but validated by the length check
	memcpy(&usType, IOExpanderMsg->abData, sizeof(usType));
	static const uint8_t abGoStatusLengths[GO_STATUS_END_OF_LIST] = {3};	// Must be min of 2 for the type

	if ((usType < GO_STATUS_END_OF_LIST) && (abGoStatusLengths[usType] == IOExpanderMsg->bLength))
	{
		switch (usType)
		{
			case GO_STATUS_IGNITION:
			{
				//Print ignition's status to user console
				if(IOExpanderMsg->abData[2])
				{
					printf("Ignition on\n");
				}
				else
				{
					printf("Ignition off\n");
				}
				break;
			}

			default:
				// Unknown type
				break;
		}
	}
}

// ****************************************************************
// *************** END of InfoTable Message Handlers **************
// ****************************************************************

//End of file
