
#ifndef MIMEPASSTHROUGH_H_
#define MIMEPASSTHROUGH_H_

#include "Types.h"

//-----------------------------------------------------------------------------
// typedef
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Public function declarations
//-----------------------------------------------------------------------------

extern void ThirdPartyInit(IOExpanderMsg_t * IOExpanderMsg);
extern void ThirdParty_ServiceComs(void);
extern void ThirdParty_SetTransmissionStatus(bool fTransmissionResult);

#endif /* MIMEPASSTHROUGH_H_ */
