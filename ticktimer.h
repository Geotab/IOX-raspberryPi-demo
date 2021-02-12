//-----------------------------------------------------------------------------
//By using and accessing this material you agree to the terms of the Sample Code
//License Agreement found here. If you do not agree you may not access 
//or otherwise use this information.
//-----------------------------------------------------------------------------
#ifndef TICKTIMER_H
#define TICKTIMER_H

#include "Types.h"
#include <sys/time.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// User defined types
//-----------------------------------------------------------------------------

typedef struct timeval Timer_t;

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public function declarations
//-----------------------------------------------------------------------------

bool Timer_HasExpired(Timer_t UserTimer, uint32_t lExpiryMilliseconds);
void Timer_Reset(Timer_t *UserTimer);

#endif // TICKTIMER_H

// End of file

