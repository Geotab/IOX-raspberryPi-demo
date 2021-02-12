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
#include "string.h"
#include "CANRxManager.h"
#include "CANTxManager.h"
#include "InfoTable.h"
#include "NVM.h"
#include <stdio.h>

//-----------------------------------------------------------------------------
// Macros
//-----------------------------------------------------------------------------

#define GO_TO_IOX_MULTI_FRAME_BUFFER_SIZE 120

//-----------------------------------------------------------------------------
// User defined types
//-----------------------------------------------------------------------------

typedef struct
{
	uint8_t abLogBuffer[GO_TO_IOX_MULTI_FRAME_BUFFER_SIZE];			// data buffer
	uint8_t bLogType;			// type of message (tGoMultiFrameMessageTypes)
	uint8_t bExpectedPacket;	// packet/frame # expected to receive next time
	uint16_t uiBytes;			// # bytes received so far
	uint16_t uiExpectedBytes;	// Total # data bytes expected as stated in frame 0

} tGO_MultiframeMessage;

//-----------------------------------------------------------------------------
// Module variable declarations
//-----------------------------------------------------------------------------

tGO_MultiframeMessage mMultiframeMessage;

//-----------------------------------------------------------------------------
// Module function declarations
//-----------------------------------------------------------------------------

static bool CANRxManager_ProcessMultiFrameMessages(IOExpanderMsg_t * IOExpanderMsg);

//-----------------------------------------------------------------------------
// Module constants
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Function definitions
//-----------------------------------------------------------------------------

// ****************************************************************
// *********************** STATIC METHODS *************************
// ****************************************************************

static bool CANRxManager_ProcessMultiFrameMessages(IOExpanderMsg_t * IOExpanderMsg)
{
	bool fLogComplete = false;		// Default message not yet completed.
	uint16_t uiNumBytes;			// # data bytes to copy from current frame
	tMultiframeLog * pLogFrame = (tMultiframeLog *)&IOExpanderMsg->abData;

	// A frame 0 is received
	if (pLogFrame->bFrameCounter == 0)
	{
		mMultiframeMessage.bLogType = pLogFrame->Frame.Zero.bType;

		mMultiframeMessage.uiExpectedBytes = pLogFrame->Frame.Zero.uiLength;
		if (pLogFrame->Frame.Zero.uiLength > sizeof(mMultiframeMessage.abLogBuffer))	// message too long
		{
			// message will have more bytes than our buffer can hold
			uint8_t bData[5];
			bData[0] = pLogFrame->Frame.Zero.bType;
			memcpy(&bData[1], (uint8_t *)&pLogFrame->Frame.Zero.uiLength, 2);

			uint16_t uiBufferSize = sizeof(mMultiframeMessage.abLogBuffer);
			memcpy(&bData[3], &uiBufferSize, 2);

			mMultiframeMessage.uiExpectedBytes = sizeof(mMultiframeMessage.abLogBuffer);	// truncate
		}

		uiNumBytes = min(mMultiframeMessage.uiExpectedBytes, 4);	// max 4 bytes to copy from frame 0

		memset(mMultiframeMessage.abLogBuffer, 0, sizeof(mMultiframeMessage.abLogBuffer));	// clear  buffer
		memcpy(mMultiframeMessage.abLogBuffer, pLogFrame->Frame.Zero.abData, uiNumBytes);
		mMultiframeMessage.uiBytes = uiNumBytes;	// # bytes received

		if (mMultiframeMessage.uiBytes >= mMultiframeMessage.uiExpectedBytes)
		{
			// all data received. This is actually a single frame
			mMultiframeMessage.bExpectedPacket = 0;	// setup to receive new message
			fLogComplete = true;								// complete message now available
		}
		else
		{
			// more data expected
			mMultiframeMessage.bExpectedPacket = 1;	// setup to receive frame 1
		}
	}

	// an expected frame is received
	else if (pLogFrame->bFrameCounter == mMultiframeMessage.bExpectedPacket)
	{
		// calculate # data bytes to copy from this frame
		uiNumBytes = mMultiframeMessage.uiExpectedBytes - mMultiframeMessage.uiBytes;	// # data bytes yet to come
		uiNumBytes = min(uiNumBytes, 7);		// max 7 bytes to copy from this frame

		// read data from this frame
		memcpy(&mMultiframeMessage.abLogBuffer[mMultiframeMessage.uiBytes], pLogFrame->Frame.N.abData, uiNumBytes);
		mMultiframeMessage.uiBytes += uiNumBytes;

		if (mMultiframeMessage.uiBytes >= mMultiframeMessage.uiExpectedBytes)
		{
			// all data received
			mMultiframeMessage.bExpectedPacket = 0;	// setup to receive new message
			fLogComplete = true;								// complete message now available
		}
		else
		{
			// more data expected
			mMultiframeMessage.bExpectedPacket++;	// setup to receive the next frame
		}
	}

	// An unexpected frame is received, abort.
	else
	{
		mMultiframeMessage.bExpectedPacket = 0;		// setup to receive new message
	}

	return (fLogComplete);
}

// ****************************************************************
// ************** START of RxManager Message Handlers *************
// ****************************************************************

void CANRxManager_MultiFrameMesssage(IOExpanderMsg_t * IOExpanderMsg)
{
	int i;

	//Check if all frames of data are received
	if (CANRxManager_ProcessMultiFrameMessages(IOExpanderMsg))
	{
		printf("Received message_id = 0x%x: Multi-Frame Data from GO - ", MSG_ID_GO_MULTIFRAME_DATA);

		//Print to user console Log type
		printf("0x%x ", mMultiframeMessage.bLogType);

		//Print to user console payload frames
		for(i = 0; i < mMultiframeMessage.uiBytes; i++)
		{
			printf("0x%x ", mMultiframeMessage.abLogBuffer[i]);
		}
		printf("\n");
	}
}

// ****************************************************************
// *************** END of RxManager Message Handlers **************
// ****************************************************************

//End of file
