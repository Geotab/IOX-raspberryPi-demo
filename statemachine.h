//-----------------------------------------------------------------------------
//By using and accessing this material you agree to the terms of the Sample Code
//License Agreement found here. If you do not agree you may not access 
//or otherwise use this information.
//-----------------------------------------------------------------------------
#ifndef STATE_MACHINE_H
#define STATE_MACHINE_H

#include "IOExpander.h"
//-----------------------------------------------------------------------------
// User defined types
//-----------------------------------------------------------------------------
typedef void (* StateMachineFunction_t)(bool fChanged);

typedef struct
{
	bool fChanged; // Indication whether the state machine state has changed
	StateMachineFunction_t pfnState; // The current state of the state machine
} StateMachine_t;

//-----------------------------------------------------------------------------
// Public functions declarations
//-----------------------------------------------------------------------------
extern void SM_Init(void);
extern void SM_Add(StateMachineFunction_t pfnStartupState);
extern void SM_Run(void);
extern void SM_SetNextState(StateMachineFunction_t pfnNextState);

#endif  // STATE_MACHINE_H

// End of file


