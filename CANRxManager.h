//-----------------------------------------------------------------------------
//By using and accessing this material you agree to the terms of the Sample Code
//License Agreement found here. If you do not agree you may not access 
//or otherwise use this information.
//-----------------------------------------------------------------------------
#ifndef CAN_RX_MANAGER_H
#define CAN_RX_MANAGER_H

#include "Types.h"
#include "IOExpander.h"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------


//-----------------------------------------------------------------------------
// User defined types
//-----------------------------------------------------------------------------

typedef struct __attribute__((packed))
{
	uint8_t bType;			// message type (IOX_MULTI_FRAME_LOG_...)
	uint16_t uiLength;		// Total # data bytes entire message contains
	uint8_t abData[4];		// data bytes
}
tMultiFrameLogPacket_0_Data;

typedef struct __attribute__((packed))
{
	uint8_t abData[ 7 ];
}
tMultiFrameLogPacket_n_Data;

typedef struct __attribute__((packed))
{
	uint8_t bFrameCounter;

	union __attribute__((packed))
	{
		tMultiFrameLogPacket_0_Data Zero;
		tMultiFrameLogPacket_n_Data N;
	}
	Frame;
}
tMultiframeLog;

typedef enum
{
	GO_MULTI_FRAME_DATA_GO_DEVICE_STATUS_V2 = 2,
	GO_MULTI_FRAME_DATA_VIN = 3,
} tGoMultiFrameMessageTypes;

//-----------------------------------------------------------------------------
// Public function declarations
//-----------------------------------------------------------------------------

void CANRxManager_MultiFrameMesssage(IOExpanderMsg_t * IOExpanderMsg);

#endif	// CAN_RX_MANAGER_H

// End of file
