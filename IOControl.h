//-----------------------------------------------------------------------------
//By using and accessing this material you agree to the terms of the Sample Code
//License Agreement found here. If you do not agree you may not access 
//or otherwise use this information.
//-----------------------------------------------------------------------------
#ifndef IOCONTROL_H
#define IOCONTROL_H

#include "IOExpander.h"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

#define 	MODEM_TRANSMISSION_RESULT  0

//-----------------------------------------------------------------------------
// User defined types
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public function declarations
//-----------------------------------------------------------------------------

extern void IOControl_Init(void);

// -------------------- Message Handlers ---------------------------------------
extern void IOControl_ResetHandler(IOExpanderMsg_t * Msg);
extern void IOControl_WakeupHandler(IOExpanderMsg_t * Msg);
extern void IOControl_SleepHandler(IOExpanderMsg_t * Msg);
extern void IOControl_PollHandler(IOExpanderMsg_t * Msg);
extern void IOControl_TxHandler(IOExpanderMsg_t * Msg);
extern void IOControl_ApplicationSpecificHandler(IOExpanderMsg_t * IOExpanderMsg);
// -----------------------------------------------------------------------------

#endif	// IOCONTROL_H

// End of file
