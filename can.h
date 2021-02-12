//-----------------------------------------------------------------------------
//By using and accessing this material you agree to the terms of the Sample Code
//License Agreement found here. If you do not agree you may not access 
//or otherwise use this information.
//-----------------------------------------------------------------------------
#ifndef CAN_H
#define CAN_H

#include "IOExpander.h"
#include <linux/can.h>
//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// User defined types
//-----------------------------------------------------------------------------

typedef void (* IOExpanderMsgRxHandler_t)(IOExpanderMsg_t* IOExpanderMsg);

// Type definition for frame structure within CAN message
typedef struct can_frame CANMsg_t;

//-----------------------------------------------------------------------------
// Public function declarations
//-----------------------------------------------------------------------------
extern uint8_t CAN_Init(void);
extern void CAN_SendMessage(CANMsg_t* pCANMsg);

#endif	// CAN_H

// End of file
