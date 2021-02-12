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
#include "ticktimer.h"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function definitions
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function   : Timer_HasExpired
// Purpose    : The function determines if a time out is reached
// Parameters : [I] lExpiryMilliseconds: Specifies time out value in milliseconds
//              [I] UserTimer: Structure holds timer's initial value
// Return     : In case time out is occurred returns true, otherwise returns false
// Notes      : None
//-----------------------------------------------------------------------------
bool Timer_HasExpired(Timer_t UserTimer, uint32_t lExpiryMilliseconds)
{
	struct timeval timerTick;
	gettimeofday(&timerTick, NULL);

	long lS = timerTick.tv_sec - UserTimer.tv_sec;
	long luS = timerTick.tv_usec - UserTimer.tv_usec;	// This needs to be signed

	if ((lS * 1000 + luS / 1000) >= lExpiryMilliseconds)
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Function   : Timer_Reset
// Purpose    : Initialise the timer
// Parameters : [O] pUserTimer: Pointer to the structure that holds the number
//                  of seconds and microseconds since the Epoch which is updated by
//                  Linux gettimeofday() function
// Return     : None
// Notes      : None
//-----------------------------------------------------------------------------
void Timer_Reset(Timer_t *pUserTimer)
{
	struct timeval timerTick;
	gettimeofday(&timerTick, NULL);

	*pUserTimer = timerTick;
}

// End of file
