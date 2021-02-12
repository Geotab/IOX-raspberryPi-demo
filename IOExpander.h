//-----------------------------------------------------------------------------
//By using and accessing this material you agree to the terms of the Sample Code
//License Agreement found here. If you do not agree you may not access 
//or otherwise use this information.
//-----------------------------------------------------------------------------
#ifndef IOExpander_H
#define IOExpander_H

#include "Types.h"

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

#define PROD_VERSION		1
#define FW_VERSION_MAJOR	1
#define FW_VERSION_MINOR	0

// ************************ Delays ****************************

#define POLL_RESPONSE_DELAY_MIN_MS					50
#define POLL_RESPONSE_DELAY_MAX_MS					200
#define CANTX_ACK_TIMEOUT_MAX_MS					250
#define DELAY_BEFORE_SLEEP_MS						2000

// ************************************************************

// ******************** Message Masks *************************

#define IO_EXPANDER_RESERVED					0x1FC00000
#define IO_EXPANDER_ID_MASK						0x0000FFFF
#define IO_EXPANDER_COMMAND_MASK				0x003F0000

// ************************************************************

// ******************** IO EXPANDER Messages ******************

#define MSG_ID_RESET							0x00
#define MSG_ID_POLL								0x01
#define MSG_ID_POLL_RESPONSE					0x02
#define MSG_ID_ADDITIONAL_INFO					0x03
#define MSG_ID_WAKEUP							0x04
#define MSG_ID_SLEEP							0x05
#define MSG_ID_TX_DATA							0x0B
#define MSG_ID_RX_DATA							0x0C
#define MSG_ID_ACK								0x14
#define MSG_ID_SINGLE_FRAME_LOG_DATA			0x1D
#define MSG_ID_MULTI_FRAME_LOG_DATA				0x1E
#define MSG_ID_IOX_STATUS_INFORMATION			0x25
#define MSG_ID_GO_STATUS_INFORMATION			0x26
#define MSG_ID_GO_MULTIFRAME_DATA     			0x27
#define MSG_ID_APPLICATION_SPECIFIC_DATA		0x1C

// ************************************************************

// ********************** IOX Types ***************************

#define IOX_TYPE_TEST 							0x9A

// ************************************************************

// ****************** Hardware Versions ***********************

#define IOX_HARDWARE_VERSION_TEST		1

// ************************************************************

//-----------------------------------------------------------------------------
// User defined types
//-----------------------------------------------------------------------------

typedef struct
{
	uint16_t	iSerialNumber;
	uint8_t		bMsgType;
	uint8_t		bLength;

	uint8_t		abData[8];

	bool		fACKRequired;
} IOExpanderMsg_t;

typedef struct
{
	uint32_t ulSerialNumber;
	uint8_t	bHardwareVersion;
	uint8_t bIoxType;

} tIOX;

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

extern tIOX gIox;

//-----------------------------------------------------------------------------
// Public function declarations
//-----------------------------------------------------------------------------

#endif	// IOExpander_H

// End of file
