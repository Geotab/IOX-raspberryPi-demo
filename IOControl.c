//-----------------------------------------------------------------------------
//By using and accessing this material you agree to the terms of the Sample Code
//License Agreement found here. If you do not agree you may not access 
//or otherwise use this information.
//-----------------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------------
#include "Types.h"
#include "string.h"
#include "IOExpander.h"
#include "statemachine.h"
#include "startup.h"
#include "NVM.h"
#include "CANTxManager.h"
#include "ticktimer.h"
#include "PRNG.h"
#include "MessageBuilder.h"
#include "IOControl.h"
#include "InfoTable.h"
#include "can.h"
#include <stdio.h>
#include "MimePassthrough.h"
//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------
#define SHA_VERSION		0x12345678

//-----------------------------------------------------------------------------
// User defined types
//-----------------------------------------------------------------------------

typedef union
{
	uint8_t bAll;

	struct
	{
		uint8_t	fGoingToSleep			: 1;
		uint8_t	fJustWokeUp				: 1;
		uint8_t	fCanActivity			: 1;

		uint8_t							: 5;

	} Single;
} IOControlFlags_t;

typedef union
{
	uint16_t iAll;

	struct
	{
		uint16_t fSendAdditionalInfo	: 1;
		uint16_t fSleep					: 1;
		uint16_t fWakeup				: 1;
		uint16_t fSendPollResponse		: 1;
		uint16_t						: 10;

	} Single;
} IOControlCommands_t;

typedef struct
{
	Timer_t							Timer;
	Timer_t							SleepTimer;

	uint32_t						iRandomDelay;

	StateMachineFunction_t			pfReturnState;

	IOControlFlags_t				Flags;

	IOControlCommands_t				Commands;

} IOControl_t;

//-----------------------------------------------------------------------------
// Module variables
//-----------------------------------------------------------------------------

uint8_t gfGoHandshakeAcked;

static IOControl_t mIOControl;

//-----------------------------------------------------------------------------
// Module function declarations
//-----------------------------------------------------------------------------

static void IOControl_SMWaitForPoll(bool fIsInit);
static void IOControl_SMConfigure(bool fIsInit);
static void IOControl_SMIdle(bool fIsInit);
static void IOControl_SMSendPollResponse(bool fIsInit);
static void IOControl_SMSendAdditionalInfo(bool fIsInit);

//-----------------------------------------------------------------------------
// Module constants
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function   : IOControl_Init
// Purpose    : Initialise the IOControl module
// Parameters : None
// Return     : None
// Notes      : None
//-----------------------------------------------------------------------------
void IOControl_Init(void)
{
	SM_Add(IOControl_SMWaitForPoll);
}

// ****************************************************************
// *********************** STATIC METHODS *************************
// ****************************************************************

// ****************************************************************
// ******************** START of IO Control SM ********************
// ****************************************************************

static void IOControl_SMWaitForPoll(bool fIsInit)
{
	if (fIsInit)
	{
		mIOControl.pfReturnState = IOControl_SMWaitForPoll;
	}

	if (mIOControl.Commands.Single.fSendPollResponse)
	{
		SM_SetNextState(IOControl_SMSendPollResponse);
		return;
	}

	if (gfGoHandshakeAcked)
	{
		SM_SetNextState(IOControl_SMConfigure);
		return;
	}
}

static void IOControl_SMConfigure(bool fIsInit)
{
	mIOControl.pfReturnState = IOControl_SMIdle;

	mIOControl.Commands.iAll = 0;	// reset commands and buffers again because of messages that could have been received (not flags)

	mIOControl.Commands.Single.fSendAdditionalInfo = true;

	CANTxManager_Reset();

	SM_SetNextState(IOControl_SMIdle);
}

static void IOControl_SMIdle(bool fIsInit)
{
	if (mIOControl.Commands.Single.fSendAdditionalInfo)
	{
		mIOControl.Commands.Single.fSendAdditionalInfo = 0;

		SM_SetNextState(IOControl_SMSendAdditionalInfo);
		return;
	}

	if (mIOControl.Commands.Single.fSleep)
	{
		mIOControl.Commands.Single.fSleep = 0;

		Timer_Reset(&mIOControl.Timer);
		mIOControl.Flags.Single.fGoingToSleep = 1;
	}

	if ((mIOControl.Flags.Single.fGoingToSleep) && (Timer_HasExpired(mIOControl.Timer, DELAY_BEFORE_SLEEP_MS)))
	{
		mIOControl.Flags.Single.fGoingToSleep = 0;

		printf("Received message_id = 0x%x: Causes all IOXs to go into sleep mode\n", MSG_ID_SLEEP);

		return;
	}

	if (mIOControl.Commands.Single.fSendPollResponse)
	{
		SM_SetNextState(IOControl_SMSendPollResponse);
		return;
	}
}

static void IOControl_SMSendAdditionalInfo(bool fIsInit)
{
	uint8_t bErrorCondition = 0;
	uint8_t bHardwareVersion = gIox.bHardwareVersion;

	IOExpanderMsg_t Msg;
	MB_AdditionalInfoMsg(&Msg, SHA_VERSION, PROD_VERSION, bErrorCondition, bHardwareVersion, true);
	CANTxManager_AddMessage(&Msg);

	SM_SetNextState(mIOControl.pfReturnState);
}

static void IOControl_SMSendPollResponse(bool fIsInit)
{
	if (fIsInit)
	{
		mIOControl.Commands.Single.fSendPollResponse = 0;
		Timer_Reset(&mIOControl.Timer);

		mIOControl.iRandomDelay = PRNG_GetNext(POLL_RESPONSE_DELAY_MIN_MS, POLL_RESPONSE_DELAY_MAX_MS);
	}

	if (Timer_HasExpired(mIOControl.Timer, mIOControl.iRandomDelay))
	{
		IOExpanderMsg_t Msg;
		uint8_t bResetReason = 0;

		MB_PollResponse(&Msg, !gfGoHandshakeAcked
		                , mIOControl.Flags.Single.fGoingToSleep
		                , mIOControl.Flags.Single.fJustWokeUp
		                , bResetReason
		                , true);

		if (mIOControl.Flags.Single.fJustWokeUp)
		{
			mIOControl.Flags.Single.fJustWokeUp = 0;
		}

		CANTxManager_AddMessage(&Msg);

		SM_SetNextState(mIOControl.pfReturnState);
	}
}

// ****************************************************************
// ********************* END of IO Control SM *********************
// ****************************************************************

// ****************************************************************
// ************* START of IO Control Message Handlers *************
// ****************************************************************

void IOControl_ResetHandler(IOExpanderMsg_t * Msg)
{
	printf("Received message_id = 0x%x: Reset IOX\n", MSG_ID_RESET);

}

void IOControl_WakeupHandler(IOExpanderMsg_t * Msg)
{
	printf("Received message_id = 0x%x: Wakes up all the IOXs from sleep mode\n", MSG_ID_WAKEUP);
	mIOControl.Flags.Single.fJustWokeUp = 1;
	mIOControl.Commands.Single.fWakeup = 1;
}

void IOControl_SleepHandler(IOExpanderMsg_t * Msg)
{
	mIOControl.Commands.Single.fSleep = 1;
}

void IOControl_PollHandler(IOExpanderMsg_t * Msg)
{
	printf("Received message_id = 0x%x: Poll request\n", MSG_ID_POLL);

	mIOControl.Commands.Single.fSendPollResponse = 1;
}

void IOControl_TxHandler(IOExpanderMsg_t * Msg)
{
	int i;

	printf("Received message_id = 0x%x: TX Data from GO - ", MSG_ID_TX_DATA);

	for(i = 0; i < Msg->bLength; i++)
	{
		printf("0x%x ", Msg->abData[i]);
	}
	printf("\n");
}

/**
 * IOControl_ApplicationSpecificHandler:
 * To process Application-Specific Data (0x1C) message coming from GO.
 * @param IOExpanderMsg : CAN message received from GO.
 * Refer to IO Expander Messages document - Application Specific Data (0x1C)
 */
void IOControl_ApplicationSpecificHandler(IOExpanderMsg_t * IOExpanderMsg)
{
	uint8_t bType = IOExpanderMsg->abData[0];

	printf("Received message_id = 0x%x: Application Specific Data from GO\n", MSG_ID_APPLICATION_SPECIFIC_DATA);

	if (bType == MODEM_TRANSMISSION_RESULT)
	{
		ThirdParty_SetTransmissionStatus(IOExpanderMsg->abData[1]);

		//Print modem transmission result
		if(IOExpanderMsg->abData[1] == 0x01)
		{
			printf("Modem transmission result - Accepted\n");
		}
		else if(IOExpanderMsg->abData[1] == 0x00)
		{
			printf("Modem transmission result - Rejected\n");
		}
	}

}

// ****************************************************************
// ************** END of IO Control Message Handlers **************
// ****************************************************************

// End of file
