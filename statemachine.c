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
#include "statemachine.h"
#include "IOExpander.h"
#include "string.h"
#include <time.h>
#include <sys/types.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

// Maximum number of state machines that can exist. Note that this number must
// be able to fit into the type used for indexing the state machines (which is
// currently a uint8_t).

#define STATE_MACHINE_MAX_NUM   20

#define SM_LOOPS_UNTIL_DELAY	2

//-----------------------------------------------------------------------------
// Module variables
//-----------------------------------------------------------------------------

static StateMachine_t maSM[STATE_MACHINE_MAX_NUM];
static uint8_t mbSMCount;
static uint8_t mbSMCurrent;

//-----------------------------------------------------------------------------
// Functions definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function   : SM_Init
// Purpose    : Initialise the state machine module.
// Parameters : None
// Return     : None
// Notes      : None
//-----------------------------------------------------------------------------
void SM_Init(void)
{
	for (mbSMCount = 0; mbSMCount < STATE_MACHINE_MAX_NUM; ++mbSMCount)
	{
		maSM[mbSMCount].fChanged = false;
		maSM[mbSMCount].pfnState = NULL;
	}
	mbSMCount = 0;
}

//-----------------------------------------------------------------------------
// Function   : SM_Add
// Purpose    : Add a state machine to the state machine scheduler.
// Parameters : [I] pfnStartupState: The state in which the state machine will
//                  start up when first scheduled to execute.
// Return     : None
// Notes      : None
//-----------------------------------------------------------------------------
void SM_Add(StateMachineFunction_t pfnStartupState)
{
	if (mbSMCount < STATE_MACHINE_MAX_NUM)
	{
		maSM[mbSMCount].fChanged = true;
		maSM[mbSMCount].pfnState = pfnStartupState;
		mbSMCount++;
	}
}

//-----------------------------------------------------------------------------
// Function   : SM_Run
// Purpose    : State machine round-robin, cooperative, non-preemptive,
//              scheduler.
// Parameters : None
// Return     : None
// Notes      : None
//-----------------------------------------------------------------------------
void SM_Run(void)
{
	static StateMachine_t * pSM;
	static StateMachineFunction_t pfnCurrent;

	static uint32_t iLoopsUntilDelay = SM_LOOPS_UNTIL_DELAY;

	while (1)
	{
		for (mbSMCurrent = 0; mbSMCurrent < mbSMCount; ++mbSMCurrent)
		{
			pSM = &maSM[mbSMCurrent];
			pfnCurrent = pSM->pfnState;
			pfnCurrent(pSM->fChanged);
			if (pfnCurrent != pSM->pfnState) // state has changed
			{
				// Set the changed flag so that initialisation can be done in the next state if necessary.
				pSM->fChanged = true;
			}
			else // State remained the same
			{
				pSM->fChanged = false;
			}
		}

		if (--iLoopsUntilDelay == 0)
		{
			struct timeval t;

			iLoopsUntilDelay = SM_LOOPS_UNTIL_DELAY;

			t.tv_sec = 0;
			t.tv_usec = 4000;
			select(0, NULL, NULL, NULL, &t);	// 4000us delay (will block this process and let others run)
		}
	}
}

//-----------------------------------------------------------------------------
// Function   : SM_SetNextState
// Purpose    : Set the next state of a state machine
// Parameters : [I] pfnNextState: The next state of the state machine.
// Return     : None
// Notes      : None
//-----------------------------------------------------------------------------
void SM_SetNextState(StateMachineFunction_t pfnNextState)
{
	maSM[mbSMCurrent].pfnState = pfnNextState;
}

// End of file


