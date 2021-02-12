//-----------------------------------------------------------------------------
//By using and accessing this material you agree to the terms of the Sample Code
//License Agreement found here. If you do not agree you may not access 
//or otherwise use this information.
//-----------------------------------------------------------------------------
#ifndef PRNG_H
#define PRNG_H

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// User defined types
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public function declarations
//-----------------------------------------------------------------------------

extern void PRNG_Init(uint32_t iSeed);
extern uint32_t PRNG_GetNext(uint32_t iMin, uint32_t iMax);

#endif	// PRNG_H

// End of file
